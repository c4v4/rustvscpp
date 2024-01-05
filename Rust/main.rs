use std::env;
use std::mem;
mod instance;
use instance::Instance;


fn main() {
    let start = std::time::Instant::now();
    let mut t1 = start;
    let args: Vec<String> = env::args().collect();
    println!("Command line: {:?}", args);

    let inst = Instance::from_file(&args[1]);
    println!(
        "Name: {}, Dist: {}, Size: {}",
        inst.get_name(),
        inst.get_dist_type(),
        inst.size()
    );
    let mut t2 = std::time::Instant::now();
    println!("Parsing instance time: {} ms.", t2.duration_since(t1).as_millis());

    println!("NN tour:");
    let mut tour = nn_tour(&inst);
    print_tour(&inst, &tour);
    t1 = std::time::Instant::now();
    println!("Nearest neighbor greedy time: {} ms.", t1.duration_since(t2).as_millis());
    
    println!("Applying 2Opt:");
    two_opt_full(&inst, &mut tour);
    t2 = std::time::Instant::now();
    println!("Full 2-Opt time: {} ms.", t2.duration_since(t1).as_millis());

    println!("Should exit after one full loop2");
    two_opt_full(&inst, &mut tour);
    print_tour(&inst, &tour);
    t1 = std::time::Instant::now();
    println!("Check 2-Opt time: {} ms.", t1.duration_since(t2).as_millis());
    println!("Total time: {} ms.", start.elapsed().as_millis());
}

fn print_tour(inst: &Instance, tour: &[u32]) -> () {
    let mut length: i32 = 0;
    for v in 1..inst.size() {
        let l = inst.dist(tour[v - 1], tour[v]);
        length += l;
    }
    length += inst.dist(tour[inst.size() - 1], tour[0]);
    println!("Lengh: {}; ", length);
}

fn nn_tour(inst: &Instance) -> Vec<u32> {
    let n = inst.size();
    let mut tour: Vec<u32> = (0..n as u32).collect();

    for i in 0..n - 2 {
        let mut nearest = i + 1;
        let mut d_nearest = inst.dist(tour[i], tour[i + 1]);
        for j in i + 2..n {
            let d_candidate = inst.dist(tour[i], tour[j]);
            if d_candidate < d_nearest {
                nearest = j;
                d_nearest = d_candidate;
            }
        }
        tour.swap(i + 1, nearest);
    }
    return tour;
}

fn two_opt_full(inst: &Instance, tour: &mut Vec<u32>) -> () {
    let n = inst.size();
    let mut ext_pass_gain = 1;
    let mut skipbits = vec![false; n];
    while ext_pass_gain > 0 {
        println!("loop 1");
        ext_pass_gain = 0;
        let mut pass_gain: i32 = 1;
        while pass_gain > 0 {
            println!("  loop 2");
            pass_gain = 0;
            for i in 0..n-2 {
                if (!skipbits[i]){
                    pass_gain += i_loop(&inst, tour, n, i, &mut skipbits);
                }
            }
            ext_pass_gain += pass_gain;
        }
    } 
}

fn i_loop(inst: &Instance, tour: &mut Vec<u32>, n: usize, i: usize, skipbits: &mut std::vec::Vec<bool>) -> i32 {
    
    let mut i_gain = 0;
    for j in i + 2..n {
        let gain = two_opt_gain(inst, tour, i, j);
        if (gain > 0) {
            i_gain += gain;
            two_opt_apply(tour, i, j);
            skipbits[j] = false;
        }
    }

    if (i_gain == 0) {
        skipbits[i] = false;
    }

    return i_gain;
}

fn two_opt_gain(inst: &Instance, tour: &[u32], i: usize, j: usize) -> i32 {
    let n = inst.size();
    let jnext = if j != n - 1 { j + 1 } else { 0 };
    let added = inst.dist(tour[i], tour[j]) + inst.dist(tour[i + 1], tour[jnext]);
    let removed = inst.dist(tour[i], tour[i + 1]) + inst.dist(tour[j], tour[jnext]);
    return removed - added;
}

fn two_opt_apply(tour: &mut Vec<u32>, i: usize, j: usize) -> () {
    tour[i + 1 .. j + 1].reverse();
}

fn two_opt_apply2(tour: &mut Vec<u32>, i: usize, j: usize) -> () {
    let beg = i + 1;
    let end = j + 1;
    let sz = (end - beg) / 2;

    let (buf1, buf2) = tour.split_at_mut(end - sz);
    let b1_iter = buf1[beg..beg + sz].iter_mut();
    let b2_iter = buf2[0..sz].iter_mut().rev();
    for it in b1_iter.zip(b2_iter) {
        mem::swap(&mut *it.0, &mut *it.1);
    }
}

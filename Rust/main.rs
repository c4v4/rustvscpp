use std::env;

mod instance;

use instance::Instance;

const MASK: u32 = 0x80000000;

macro_rules! flag_clear {
    ($x:expr,$y:expr) => {
        $x[$y as usize] & !MASK
    };
}

macro_rules! flag_off {
    ($x:expr,$y:expr) => {
        $x[$y as usize] = flag_clear!($x, $y)
    };
}

macro_rules! flag_on {
    ($x:expr,$y:expr) => {
        $x[$y as usize] |= MASK;
    };
}

macro_rules! flag_test {
    ($x:expr,$y:expr) => {
        ($x[$y as usize] & MASK != 0)
    };
}

fn main() {
    let args: Vec<String> = env::args().collect();
    println!("Command line: {:?}", args);

    let inst = Instance::from_file(&args[1]);

    println!(
        "Name: {}, Dist: {}, Size: {}",
        inst.get_name(),
        inst.get_dist_type(),
        inst.size()
    );

    println!("NN tour:");
    let mut tour = nn_tour(&inst);
    print_tour(&inst, &tour);

    println!("Applying 2Opt:");
    two_opt_full(&inst, &mut tour);
    println!("Should exit after one full loop2");
    two_opt_full(&inst, &mut tour);
    print_tour(&inst, &tour);
}

fn print_tour(inst: &Instance, tour: &[u32]) -> () {
    let mut length: i32 = 0;
    //println!("{:2?}", tour);
    for v in 1..(inst.size()) {
        let l = inst.dist(tour[(v - 1) as usize], tour[(v) as usize]);
        length += l;
        //print!("{} {} ", l, if v < inst.size() - 1 { "+" } else { "=" });
    }
    println!("Lengh: {}; ", length);
}

fn nn_tour(inst: &Instance) -> Vec<u32> {
    let mut tour: Vec<u32> = vec![0; inst.size() as usize];
    let n = inst.size();
    for v in 0..n {
        tour[(v) as usize] = v;
    }

    for i in 0..n - 2 {
        let mut nearest = i + 1;
        let mut d_nearest = inst.dist(tour[(i) as usize], tour[(i + 1) as usize]);
        for j in (i + 2)..n {
            let d_candidate = inst.dist(tour[(i) as usize], tour[(j) as usize]);
            if d_candidate < d_nearest {
                nearest = j;
                d_nearest = d_candidate;
            }
        }
        let temp = tour[(i + 1) as usize];
        tour[(i + 1) as usize] = tour[(nearest) as usize];
        tour[(nearest) as usize] = temp;
    }
    return tour;
}

fn two_opt_full(inst: &Instance, tour: &mut Vec<u32>) -> () {
    let n = inst.size();
    let mut skip_flag_test = 2;
    while {
        println!("loop 1");
        let mut ext_pass_gain: i32 = 0;
        while {
            println!("  loop 2");
            let mut pass_gain: i32 = 0;
            for i in 0..(n - 1) {
                if !(skip_flag_test > 0) {
                    let cond = !flag_test!(tour, i) || !flag_test!(tour, i + 1);
                    flag_off!(tour, i);
                    if i + 1 == n - 1 {
                        flag_off!(tour, i + 1);
                    }
                    if cond {
                        continue;
                    }
                }

                let mut i_gain: i32 = 0;
                let mut j = i + 2;
                while j < n {
                    let jnext = if j != n - 1 { j + 1 } else { 0 };

                    let gain = two_opt_gain(&inst, &tour, i, j);
                    if gain > 0 {
                        i_gain += gain;
                        two_opt_apply(&inst, tour, i, j);
                        flag_on!(tour, j);
                        flag_on!(tour, jnext);
                    }
                    j += 1;
                }

                if i_gain > 0 {
                    flag_on!(tour, i);
                    flag_on!(tour, i + 1);
                    pass_gain += i_gain;
                }
            }
            if pass_gain > 0 {
                ext_pass_gain += pass_gain;
            }
            skip_flag_test = 0;
            pass_gain > 0
        } {}
        skip_flag_test = 1;
        ext_pass_gain > 0
    } {}
}

fn two_opt_gain(inst: &Instance, tour: &[u32], i: u32, j: u32) -> i32 {
    let n = inst.size();
    let jnext = if j != n - 1 { j + 1 } else { 0 };
    let added = inst.dist(flag_clear!(tour, i), flag_clear!(tour, j))
        + inst.dist(flag_clear!(tour, i + 1), flag_clear!(tour, jnext));
    let removed = inst.dist(flag_clear!(tour, i), flag_clear!(tour, i + 1))
        + inst.dist(flag_clear!(tour, j), flag_clear!(tour, jnext));
    return removed - added;
}

fn two_opt_apply(inst: &Instance, tour: &mut Vec<u32>, i: u32, j: u32) -> () {
    flip_n(tour, i + 1, j, (j - i) / 2);
}

fn flip_n(tour: &mut [u32], beg: u32, end: u32, n: u32) {
    let mid = end + 1 - n;
    let (buf1, buf2) = tour.split_at_mut(mid as usize);
    flip_internal(&mut buf1[(beg as usize)..], buf2, n as isize);
}

pub fn flip_internal(buf1: &mut [u32], buf2: &mut [u32], n: isize) {
    unsafe {
        let b1: *mut _ = buf1.as_mut_ptr();
        let b2: *mut _ = buf2.as_mut_ptr();
        let mut k: isize = 0;
        while k < n {
            let temp = *b1.offset(k);
            *b1.offset(k) = *b2.offset(n - 1 - k);
            *b2.offset(n - 1 - k) = temp;
            k += 1;
        }
    }
}

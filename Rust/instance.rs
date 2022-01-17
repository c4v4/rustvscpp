use std::fs::File;
use std::io::{self, BufRead};
use std::path::Path;

pub const THRESHOLD: u32 = 0x7FFFFFFF;

#[derive(Copy, Clone)]
pub struct Coords {
    x: f64,
    y: f64,
}

#[derive(Copy, Clone)]
pub struct Edge {
    pub v: usize,
    pub u: usize,
}

type Dist = fn(Coords, Coords) -> i32;

pub fn dist_one(v: Coords, u: Coords) -> i32 {
    return 1;
}

pub fn dist_euc2d(v: Coords, u: Coords) -> i32 {
    let dx = v.x - u.x;
    let dy = v.y - u.y;
    return ((dx * dx + dy * dy).sqrt() + 0.5) as i32;
}

pub fn dist_att(v: Coords, u: Coords) -> i32 {
    let xd = v.x - u.x;
    let yd = v.y - u.y;
    let rij = ((xd * xd + yd * yd) / 10.0).sqrt();
    let tij = (rij + 0.5) as i32;
    return if (tij as f64) < rij { tij + 1 } else { tij };
}

fn read_lines<P>(filename: P) -> io::Result<io::Lines<io::BufReader<File>>>
where
    P: AsRef<Path>,
{
    let file = File::open(filename)?;
    Ok(io::BufReader::new(file).lines())
}

fn dist_offset(n: u32, x: u32, y: u32) -> usize {
    let min = if x > y { y } else { x };
    let max = if x < y { y } else { x };
    let d = max - min;
    let b1 = (!0) * (d > n / 2) as u32;
    let b2 = (!0) * (d <= n / 2) as u32;
    let row = (((n - d) & b1) | (d & b2)) - 1;
    let col = if b1 != 0 { max } else { min };
    return (row * n + col) as usize;
}

#[derive(Clone)]
pub struct Instance {
    name: String,
    dist_type: String,
    coords: Vec<Coords>,
    dists: Vec<i32>,
    d: Dist,
}

impl Instance {
    pub fn from_file(file_path: &str) -> Instance {
        let mut inst = Instance {
            name: "".to_string(),
            dist_type: "".to_string(),
            coords: vec![],
            dists: vec![],
            d: dist_one,
        };
        if let Ok(lines) = read_lines(file_path) {
            let mut coord_to_read = 0;
            let mut dimension = -1;
            for line in lines {
                let l = line.unwrap();
                if coord_to_read > 0 {
                    coord_to_read -= 1;
                    //println!("{}", l);
                    let splitted: Vec<&str> = l.split_whitespace().collect();
                    let id: usize = splitted[0].parse::<usize>().unwrap();
                    let x: f64 = splitted[1].parse::<f64>().unwrap();
                    let y: f64 = splitted[2].parse::<f64>().unwrap();
                    inst.coords[id - 1] = Coords { x: x, y: y };
                } else {
                    let splitted: Vec<&str> = l.split(":").collect();
                    let keyword: &str = splitted[0];
                    match keyword.trim() {
                        "NAME" => {
                            let value: &str = &splitted[1..].join("");
                            inst.name = value.to_string();
                            println!("Name: {}", inst.name);
                        }
                        "DIMENSION" => {
                            let value: &str = &splitted[1..].join("");
                            dimension = value.trim().parse::<i32>().unwrap();
                            inst.coords
                                .resize(dimension as usize, Coords { x: 0.0, y: 0.0 });
                            println!("Dimension: {}", value);
                        }
                        "EDGE_WEIGHT_TYPE" => {
                            let value: &str = &splitted[1..].join("");
                            println!("Distance function: {}", value);
                            match value.trim() {
                                "EUC_2D" => inst.d = dist_euc2d,
                                "ATT" => inst.d = dist_att,
                                _ => eprint!("Distance function not supported\n"),
                            }
                        }
                        "NODE_COORD_TYPE" => {
                            let value: &str = &splitted[1..].join("");
                            println!("Coordinates type: {}", value);
                        }
                        "NODE_COORD_SECTION" => {
                            assert_ne!(dimension, -1, "Dimension not read yet!");
                            coord_to_read = dimension;
                            println!("Coords section:");
                        }
                        "DEPOT_SECTION" => {
                            println!("Depot section:");
                        }
                        _ => {
                            eprint!("Keyword not recognized! {}\n", keyword)
                        }
                    }
                }
            }
            /* let n = if ((inst.size() / 2) as usize * inst.size() as usize) < THRESHOLD as usize {
                (inst.size() / 2) * inst.size()
            } else {
                THRESHOLD
            };
            inst.dists.resize(n as usize, 0);
            for v in 0..(inst.size()) {
                for u in 0..v {
                    let off = dist_offset(inst.size(), v, u);
                    if off < THRESHOLD as usize {
                        inst.dists[off] =
                            (inst.d)(inst.coords[v as usize], inst.coords[u as usize]);
                    }
                }
            } */
        }
        return inst;
    }

    pub fn get_name(&self) -> &str {
        return self.name.as_ref();
    }
    pub fn get_dist_type(&self) -> &str {
        return self.dist_type.as_ref();
    }

    pub fn size(&self) -> u32 {
        return self.coords.len() as u32;
    }

    pub fn get_x(&self, i: u32) -> f64 {
        return self.coords[i as usize].x;
    }

    pub fn get_y(&self, i: u32) -> f64 {
        return self.coords[i as usize].y;
    }

    pub fn dist(&self, u: u32, v: u32) -> i32 {
        return dist_euc2d(self.coords[v as usize], self.coords[u as usize]);
        /*  let off = dist_offset(self.size(), v, u);
        return if off < THRESHOLD as usize {
            self.dists[off]
        } else {
            (self.d)(self.coords[v as usize], self.coords[u as usize])
        }; */
    }
}

#!/bin/bash

function runRust {
    cd Rust
    echo "Rust"
    RUSTFLAGS="-C target-cpu=native -C llvm-args=-ffast-math" cargo run --release ../instances/pla33810.tsp
    cd -
}

function runCpp {
    cd Cpp
    echo "Cpp"
    g++ -o twoopt main.cpp -O3 -march=native -mtune=native -ffast-math -lfmt && ./twoopt ../instances/pla33810.tsp
    cd -
}

function runZig {
    cd Zig
    echo "Zig"
    zig run -O ReleaseFast -fsingle-threaded main.zig -- ../instances/pla33810.tsp
    cd -
}

runRust
runCpp
runZig

runRust
runZig
runCpp

runCpp
runRust
runZig

runCpp
runZig
runRust

runZig
runRust
runCpp

runZig
runCpp
runRust
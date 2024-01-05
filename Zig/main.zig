const std = @import("std");
const assert = std.debug.assert;
const Allocator = std.mem.Allocator;

const Instance = @import("instance.zig").Instance;
const read_inst_file = @import("instance.zig").read_inst_file;
const print = @import("instance.zig").print;

pub fn main() !void {
    var start = try std.time.Timer.start();
    var stopwatch = try std.time.Timer.start();

    const argv = std.os.argv;
    if (argv.len < 2) {
        print("Instance path not provided\n", .{});
    }

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const alloc = gpa.allocator();

    const inst = try read_inst_file(std.mem.span(argv[1]), alloc);
    print("Name: {s}, Dist: {s}, Size: {d}\n", .{ inst.name, inst.dist_type, inst.size() });
    print("Parsing instance time: {} ms.\n", .{stopwatch.lap() / 1000000});

    print("NN tour:\n", .{});
    var tour = try nn_tour(&inst, alloc);
    print_tour(&inst, tour);
    print("Nearest neighbor greedy time: {} ms.\n", .{stopwatch.lap() / 1000000});

    print("Applying 2Opt:\n", .{});
    try two_opt_full(&inst, tour, alloc);
    print("Full 2-Opt time: {} ms.\n", .{stopwatch.lap() / 1000000});

    print("Should exit after one full loop2\n", .{});
    try two_opt_full(&inst, tour, alloc);
    print_tour(&inst, tour);
    print("Check 2-Opt time: {} ms.\n", .{stopwatch.lap() / 1000000});
    print("Total time: {} ms.\n", .{start.read() / 1000000});
}

fn print_tour(inst: *const Instance, tour: []u32) void {
    var length: i32 = 0;
    for (1..inst.size()) |v| {
        length += inst.dist(tour[v - 1], tour[v]);
    }
    length += inst.dist(tour[inst.size() - 1], tour[0]);
    print("Lengh: {};\n", .{length});
}

fn nn_tour(inst: *const Instance, allocator: Allocator) ![]u32 {
    const n = inst.size();
    var tour: []u32 = try allocator.alloc(u32, n);
    for (tour, 0..) |*item, i| {
        item.* = @intCast(i);
    }

    for (0..n - 2) |i| {
        var nearest = i + 1;
        var d_nearest = inst.dist(tour[i], tour[i + 1]);
        for (i + 2..n) |j| {
            const d_candidate = inst.dist(tour[i], tour[j]);
            if (d_candidate < d_nearest) {
                nearest = j;
                d_nearest = d_candidate;
            }
        }
        std.mem.swap(u32, &tour[i + 1], &tour[nearest]);
    }
    return tour;
}

fn two_opt_full(inst: *const Instance, tour: []u32, allocator: Allocator) !void {
    const n = inst.size();
    var ext_pass_gain: i32 = 1;

    var skipbits: []bool = try allocator.alloc(bool, n);
    for (skipbits) |*item| {
        item.* = false;
    }

    while (ext_pass_gain > 0) {
        print("loop 1\n", .{});
        ext_pass_gain = 0;
        var pass_gain: i32 = 1;
        while (pass_gain > 0) {
            print("  loop 2\n", .{});
            pass_gain = 0;
            for (0..n - 2) |i| {
                if (!skipbits[i]) {
                    pass_gain += i_loop(inst, tour, n, i, skipbits);
                }
            }
            ext_pass_gain += pass_gain;
        }
    }
}

fn i_loop(inst: *const Instance, tour: []u32, n: usize, i: usize, skipbits: []bool) i32 {
    var i_gain: i32 = 0;
    for (i + 2..n) |j| {
        const gain = two_opt_gain(inst, tour, i, j);
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

fn two_opt_gain(inst: *const Instance, tour: []u32, i: usize, j: usize) i32 {
    const n = inst.size();
    const jnext = if (j != n - 1) j + 1 else 0;
    const added = inst.dist(tour[i], tour[j]) + inst.dist(tour[i + 1], tour[jnext]);
    const removed = inst.dist(tour[i], tour[i + 1]) + inst.dist(tour[j], tour[jnext]);
    return removed - added;
}

fn two_opt_apply(tour: []u32, i: usize, j: usize) void {
    std.mem.reverse(u32, tour[i + 1 .. j + 1]);
}

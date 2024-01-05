const std = @import("std");
const assert = std.debug.assert;
const Allocator = std.mem.Allocator;

pub fn print(comptime fmt: []const u8, args: anytype) void {
    const stdout = std.io.getStdOut().writer();
    nosuspend stdout.print(fmt, args) catch return;
}

const Coord = struct { x: f32, y: f32 };

const Dist = *const fn (v: Coord, u: Coord) i32;

pub const Instance = struct {
    name: []const u8,
    dist_type: []const u8,
    coords: []Coord,
    d: Dist,

    pub fn size(self: Instance) usize {
        return self.coords.len;
    }

    pub fn dist(self: Instance, v: u32, u: u32) i32 {
        assert(v < self.size() and u < self.size());
        return self.d(self.coords[v], self.coords[u]);
    }
};

pub fn dist_one(_: Coord, _: Coord) i32 {
    return 1;
}

pub fn dist_euc2d(v: Coord, u: Coord) i32 {
    const dx: f64 = v.x - u.x;
    const dy: f64 = v.y - u.y;
    return @intFromFloat(@sqrt(dx * dx + dy * dy) + 0.5);
}

pub fn dist_att(v: Coord, u: Coord) i32 {
    const xd: f64 = v.x - u.x;
    const yd: f64 = v.y - u.y;
    const rij: f64 = @sqrt((xd * xd + yd * yd) / 10.0);
    const tij: i32 = @as(i32, @intFromFloat(rij + 0.5));
    if (tij < @as(i32, @intFromFloat(rij)))
        return tij + 1;
    return tij;
}

pub fn dist_ceil2d(v: Coord, u: Coord) i32 {
    const dx: f64 = v.x - u.x;
    const dy: f64 = v.y - u.y;
    return @as(i32, @intFromFloat(@ceil(@sqrt(dx * dx + dy * dy))));
}

const TsplibKeys = enum {
    NAME,
    DIMENSION,
    EDGE_WEIGHT_TYPE,
    NODE_COORD_TYPE,
    NODE_COORD_SECTION,
    DEPOT_SECTION,
    UNDEFINED,
};

const DistKeys = enum {
    ONE,
    EUC_2D,
    ATT,
    CEIL_2D,
    UNDEFINED,
};

pub fn read_inst_file(path: []const u8, allocator: Allocator) !Instance {
    const inst_file = try std.fs.cwd().openFile(path, .{});
    defer inst_file.close();

    var buf_reader = std.io.bufferedReader(inst_file.reader());
    var in_stream = buf_reader.reader();

    var buf: [1024]u8 = undefined;
    var inst: Instance = undefined;
    var dim: usize = undefined;
    while (try in_stream.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var tokens = std.mem.split(u8, line, " ");
        const tk = tokens.next() orelse continue; //skip empty lines

        const okeyw = std.meta.stringToEnum(TsplibKeys, tk);
        if (okeyw) |k| {
            _ = tokens.next(); // colon
            switch (k) {
                .NAME => {
                    inst.name = try allocator.dupe(u8, tokens.rest());
                    print("NAME: {s}\n", .{inst.name});
                },
                .DIMENSION => {
                    dim = try std.fmt.parseInt(usize, tokens.rest(), 10);
                    print("DIMENSION: {s}\n", .{tokens.rest()});
                },
                .EDGE_WEIGHT_TYPE => {
                    inst.dist_type = try allocator.dupe(u8, tokens.rest());
                    print("EDGE_WEIGHT_TYPE: {s}\n", .{inst.dist_type});
                    const odist = std.meta.stringToEnum(DistKeys, inst.dist_type);
                    if (odist) |d| {
                        inst.d = switch (d) {
                            .ONE => dist_one,
                            .EUC_2D => dist_euc2d,
                            .ATT => dist_att,
                            .CEIL_2D => dist_ceil2d,
                            else => undefined,
                        };
                    }
                },
                .NODE_COORD_SECTION => {
                    print("NODE_COORD_SECTION:\n", .{});
                    assert(dim > 0);
                    inst.coords = try allocator.alloc(Coord, dim);
                    for (0..dim) |i| {
                        const c_line = (try in_stream.readUntilDelimiterOrEof(&buf, '\n')).?;
                        var c_tokens = std.mem.splitScalar(u8, c_line, ' ');
                        const id = try std.fmt.parseInt(usize, c_tokens.next().?, 10);
                        const x = try std.fmt.parseFloat(f32, c_tokens.next().?);
                        const y = try std.fmt.parseFloat(f32, c_tokens.next().?);

                        inst.coords[id - 1] = Coord{ .x = x, .y = y };
                        assert(id - 1 == i);
                    }
                },
                .DEPOT_SECTION => {
                    print("DEPOT_SECTION:\n", .{});
                    const id = try std.fmt.parseInt(i32, tokens.next().?, 10); // id
                    assert(id == 1);
                },
                else => print("Keyword not recognized: {s}\n", .{tk}),
            }
        } else {
            print("Keyword not recognized: {s}\n", .{tk});
        }
    }

    return inst;
}

const std = @import("std");
const Gmi = @import("gmi_parser").Gmi;
const SGR = @import("ansi.zig").SGR;

pub fn main() !void {
    const allocator = std.heap.page_allocator;

    const args = try std.process.argsAlloc(allocator);
    defer allocator.free(args);

    if (args.len != 2) {
        std.debug.print("Usage: {s} [path to gemtext file]\n", .{args[0]});
        return;
    }

    // Step 3: Read Gemtext file from disk.
    const file_path = args[1];
    const file = try std.fs.cwd().openFile(file_path, .{});
    defer file.close();

    const content = try file.readToEndAlloc(allocator, 0); // 0 means no maximum size limit.
    defer allocator.free(content);

    // Step 4: Parse the Gemtext content.
    var gmi = Gmi.init(allocator, content);

    while (gmi.next()) |line| {
        switch (line.line_type) {
            .Text => {
                std.debug.print("{s}\n", .{line.line});
            },
            .Link => {
                std.debug.print("{s}{s} -> {s}\n", .{
                    SGR.FgBlue.ansiCode(),
                    line.line,
                    SGR.Reset.ansiCode(),
                });
            },
            .Heading1 => {
                std.debug.print("{s}{s}{s}\n", .{
                    SGR.Bold.ansiCode(),
                    line.line,
                    SGR.Reset.ansiCode(),
                });
            },
            // ... handle other line types similarly
            else => {},
        }
    }
}

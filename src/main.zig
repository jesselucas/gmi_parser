const std = @import("std");

pub const LineType = enum {
    Text,
    PreToggleBegin,
    PreToggleEnd,
    PreText,
    Link,
    LinkImg,
    Heading1,
    Heading2,
    Heading3,
    List,
    Quote,
};

pub const Line = struct {
    line_type: LineType,
    number: usize,
    line: []const u8,
};

pub const Gmi = struct {
    content: []const u8,
    parsed_line_index: usize = 0,
    position: usize,
    preformat_mode: bool,
    lines: std.ArrayList(Line),

    pub fn init(allocator: std.mem.Allocator, content: []const u8) Gmi {
        return Gmi{
            .content = content,
            .position = 0,
            .preformat_mode = false,
            .lines = std.ArrayList(Line).init(allocator),
        };
    }

    pub fn next(self: *Gmi) ?Line {
        if (self.position >= self.content.len) {
            return null;
        }

        // Find the next newline character
        const next_newline_position = std.mem.indexOf(u8, self.content[self.position..], "\n") orelse (self.content.len - self.position);
        const next_position = self.position + next_newline_position;

        // Make sure the slice is within bounds
        if (self.position > next_position or next_position > self.content.len) {
            return null;
        }

        // Extract line and update position
        const line = self.content[self.position..next_position];
        self.position = next_position + 1;
        defer self.parsed_line_index += 1;

        return self.parseLine(self.parsed_line_index, line) catch {
            return null;
        };
    }

    pub fn appendLine(self: *Gmi, new_line: Line) !void {
        try self.lines.append(new_line);
    }

    pub fn isImageLink(line: []const u8) bool {
        const image_extensions = &[_][]const u8{ "gif", "jpeg", "jpg", "png" };
        for (image_extensions) |ext| {
            if (std.mem.endsWith(u8, line, ext)) return true;
        }
        return false;
    }

    pub fn parseLine(self: *Gmi, line_number: usize, line: []const u8) !Line {
        var line_type = LineType.Text;

        if (self.preformat_mode) {
            if (line.len >= 3 and std.mem.eql(u8, line[0..3], "```")) {
                self.preformat_mode = false;
                line_type = LineType.PreToggleEnd;
            } else {
                line_type = LineType.PreText;
            }
        } else {
            switch (line[0]) {
                '=' => {
                    if (line.len > 1 and line[1] == '>') {
                        line_type = if (isImageLink(line)) LineType.LinkImg else LineType.Link;
                    }
                },
                '`' => if (line.len >= 3 and std.mem.eql(u8, line[0..3], "```")) {
                    self.preformat_mode = true;
                    line_type = LineType.PreToggleBegin;
                },
                '#' => {
                    line_type = LineType.Heading1;
                    if (line.len >= 2 and line[1] == '#') line_type = LineType.Heading2;
                    if (line.len >= 3 and std.mem.eql(u8, line[1..3], "##")) line_type = LineType.Heading3;
                },
                '*' => line_type = LineType.List,
                '>' => line_type = LineType.Quote,
                else => {},
            }
        }

        const new_line = Line{ .line_type = line_type, .number = line_number, .line = line };
        try self.appendLine(new_line);
        return new_line;
    }

    pub fn deinit(self: *Gmi) void {
        self.lines.deinit();
    }
};

test "testLines" {
    const test_allocator = std.testing.allocator;

    const content = "All the following examples are valid link lines:\n" ++
        "=> gemini://example.org/\n" ++
        "=> gemini://example.org/ An example link\n";

    var gmi = Gmi.init(test_allocator, content);
    defer gmi.deinit();

    var lines = std.ArrayList(Line).init(test_allocator);
    defer lines.deinit();

    var i: usize = 0;
    while (gmi.next()) |line| {
        try lines.append(line);
        try std.testing.expectEqual(line.number, i);
        i += 1;
    }
    try std.testing.expectEqual(lines.items.len, 3);
}

test "testHeaders" {
    const test_allocator = std.testing.allocator;
    const content = "# Header 1\n## Header 2\n### Header 3\n";
    var gmi = Gmi.init(test_allocator, content);
    defer gmi.deinit();

    var lines = std.ArrayList(Line).init(test_allocator);
    defer lines.deinit();

    const expected_types = &[_]LineType{ LineType.Heading1, LineType.Heading2, LineType.Heading3 };

    var i: usize = 0;
    while (gmi.next()) |line| {
        try lines.append(line);
        try std.testing.expectEqual(line.line_type, expected_types[i]);
        try std.testing.expectEqual(line.number, i);
        i += 1;
    }
    try std.testing.expectEqual(lines.items.len, 3);
}

test "testLinks" {
    const test_allocator = std.testing.allocator;
    const content = "=> link\n=> image.png\n= not a link\n";
    var gmi = Gmi.init(test_allocator, content);
    defer gmi.deinit();

    var lines = std.ArrayList(Line).init(test_allocator);
    defer lines.deinit();

    const expected_types = &[_]LineType{ LineType.Link, LineType.LinkImg, LineType.Text };

    var i: usize = 0;
    while (gmi.next()) |line| {
        try lines.append(line);
        try std.testing.expectEqual(line.line_type, expected_types[i]);
        try std.testing.expectEqual(line.number, i);
        i += 1;
    }
    try std.testing.expectEqual(lines.items.len, 3);
}

//    pub fn testHeaders() void {
//        const test_allocator = std.testing.allocator;
//        var gmi = Gmi.init(test_allocator);
//        const lines = &[_][]const u8{
//            "# Header 1",
//            "## Header 2",
//            "### Header 3",
//            // ... add more test lines here
//        };
//        var i: usize = 0;
//        while (i < lines.len) : (i += 1) {
//            _ = gmi.parseLine(i, lines[i]) catch unreachable;
//        }
//
//        for (gmi.lines, 0..) |line, index| {
//            const expected_type = if (index == 0) LineType.Heading1 else if (index == 1) LineType.Heading2 else LineType.Heading3;
//            std.testing.expectEqual(line.line_type, expected_type);
//        }
//        gmi.deinit();
//    }
//
//    pub fn testLinks() void {
//        const test_allocator = std.testing.allocator;
//        var gmi = Gmi.init(test_allocator);
//        const lines = &[_][]const u8{
//            "=> link",
//            "=> image.png",
//            "= not a link",
//            // ... add more test lines here
//        };
//        var i: usize = 0;
//        while (i < lines.len) : (i += 1) {
//            _ = gmi.parseLine(i, lines[i]) catch unreachable;
//        }
//
//        try std.testing.expectEqual(gmi.lines[0].line_type, LineType.Link);
//        try std.testing.expectEqual(gmi.lines[1].line_type, LineType.LinkImg);
//        try std.testing.expectEqual(gmi.lines[2].line_type, LineType.Text);
//
//        gmi.deinit();
//    }
//};
//
//// Zig test for line types
//test "testLines" {
//    const test_allocator = std.testing.allocator;
//    var gmi = Gmi.init(test_allocator);
//
//    const lines = &[_][]const u8{
//        "All the following examples are valid link lines:",
//        "=> gemini://example.org/",
//        "=> gemini://example.org/ An example link",
//        // ... (Your remaining test lines go here)
//    };
//
//    var i: usize = 0;
//    while (i < lines.len) : (i += 1) {
//        _ = gmi.parseLine(i, lines[i]) catch unreachable;
//    }
//    try std.testing.expectEqual(gmi.lines.items.len, lines.len);
//
//    gmi.deinit();
//}
//
//// Zig test for headers
//test "testHeaders" {
//    const test_allocator = std.testing.allocator;
//    var gmi = Gmi.init(test_allocator);
//
//    const lines = &[_][]const u8{
//        "# Header 1",
//        "## Header 2",
//        "### Header 3",
//        // ... (Your remaining test lines go here)
//    };
//
//    var i: usize = 0;
//    while (i < lines.len) : (i += 1) {
//        _ = gmi.parseLine(i, lines[i]) catch unreachable;
//    }
//
//    // Checks the type for each line in the tests
//    const expectedTypes = &[_]LineType{ LineType.Heading1, LineType.Heading2, LineType.Heading3 };
//    for (gmi.lines.items, 0..) |line, index| {
//        try std.testing.expectEqual(line.line_type, expectedTypes[index]);
//    }
//
//    gmi.deinit();
//}
//
//// Zig test for links
//test "testLinks" {
//    const test_allocator = std.testing.allocator;
//    var gmi = Gmi.init(test_allocator);
//
//    const lines = &[_][]const u8{
//        "=> link",
//        "=> image.png",
//        "= not a link",
//        // ... (Your remaining test lines go here)
//    };
//
//    var i: usize = 0;
//    while (i < lines.len) : (i += 1) {
//        _ = gmi.parseLine(i, lines[i]) catch unreachable;
//    }
//
//    const expectedTypes = &[_]LineType{ LineType.Link, LineType.LinkImg, LineType.Text };
//    for (gmi.lines.items, 0..) |line, index| {
//        try std.testing.expectEqual(line.line_type, expectedTypes[index]);
//    }
//
//    gmi.deinit();
//}

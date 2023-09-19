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
    content: ?[]const u8,
    parsed_line_index: usize = 0,
    position: usize,
    preformat_mode: bool,
    lines: std.ArrayList(Line),

    stream_buffer_state: StreamBufferState,

    const StreamBufferState = union(enum) {
        NoBuffer,
        HasBuffer: std.ArrayList(u8),
    };

    pub fn init(allocator: std.mem.Allocator, content: ?[]const u8) !Gmi {
        var stream_buffer_state = StreamBufferState{ .NoBuffer = {} };
        if (content == null) {
            var buffer = std.ArrayList(u8).init(allocator);
            try buffer.resize(0);
            stream_buffer_state = StreamBufferState{ .HasBuffer = buffer };
        }
        return Gmi{
            .content = content,
            .position = 0,
            .preformat_mode = false,
            .lines = std.ArrayList(Line).init(allocator),
            .stream_buffer_state = stream_buffer_state,
        };
    }

    pub fn parseChunk(self: *Gmi, chunk: []const u8) !void {
        switch (self.stream_buffer_state) {
            StreamBufferState.NoBuffer => {
                return;
            },
            StreamBufferState.HasBuffer => |*buffer| {
                try buffer.appendSlice(chunk);
            },
        }
    }

    pub fn next(self: *Gmi) ?Line {
        if (self.content) |content| { // Non-stream mode
            if (self.position >= content.len) {
                return null;
            }

            // Find the next newline character
            const next_newline_position = std.mem.indexOf(
                u8,
                content[self.position..],
                "\n",
            ) orelse (content.len - self.position);
            const next_position = self.position + next_newline_position;

            // Make sure the slice is within bounds
            if (self.position > next_position or next_position > content.len) {
                return null;
            }

            // Extract line and update position
            const line = content[self.position..next_position];
            self.position = next_position + 1;
            defer self.parsed_line_index += 1;

            return self.parseLine(self.parsed_line_index, line) catch {
                return null;
            };
        } else { // Stream mode
            switch (self.stream_buffer_state) {
                StreamBufferState.NoBuffer => {
                    // Do nothing or return null
                    return null;
                },
                StreamBufferState.HasBuffer => |*buffer| {
                    while (true) {
                        const newline_pos = std.mem.indexOf(u8, buffer.items, "\n") orelse break;

                        const line = buffer.items[0..newline_pos];
                        buffer.items = buffer.items[newline_pos + 1 ..];
                        defer self.parsed_line_index += 1;

                        const parsed_line = self.parseLine(self.parsed_line_index, line) catch {
                            continue;
                        };
                        return parsed_line;
                    }
                },
            }
        }
        return null;
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
        switch (self.stream_buffer_state) {
            StreamBufferState.NoBuffer => {},
            StreamBufferState.HasBuffer => |buffer| {
                std.debug.print("only once! {}\n", .{buffer});
                if (buffer.items.len > 0)
                    buffer.deinit();
            },
        }
    }
};

test "test lines" {
    const test_allocator = std.testing.allocator;

    const content = "All the following examples are valid link lines:\n" ++
        "=> gemini://example.org/\n" ++
        "=> gemini://example.org/ An example link\n";

    var gmi = try Gmi.init(test_allocator, content);
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

test "test headers" {
    const test_allocator = std.testing.allocator;
    const content = "# Header 1\n## Header 2\n### Header 3\n";
    var gmi = try Gmi.init(test_allocator, content);
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

test "test links" {
    const test_allocator = std.testing.allocator;
    const content = "=> link\n=> image.png\n= not a link\n";
    var gmi = try Gmi.init(test_allocator, content);
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

test "test streaming mode" {
    const test_allocator = std.testing.allocator;

    // Initialize Gmi with null content to simulate streaming
    var gmi = try Gmi.init(test_allocator, null);
    defer gmi.deinit();

    //    var lines = std.ArrayList(Line).init(test_allocator);
    //test_allocator    defer lines.deinit();

    // Simulate chunking. Assume each chunk contains a line.
    // const chunks = [_][]const u8{ "Lin", "e 1\n", "Line ", "2\n", "Line 3\n" };
    const chunks = [_][]const u8{ "Line 1\n", "Line 2\n", "Line 3\n" };

    var i: usize = 0;
    for (chunks) |chunk| {
        std.debug.print("chunk! {s}\n", .{chunk});
        try gmi.parseChunk(chunk);

        while (gmi.next()) |line| {
            std.debug.print("line! {s}\n", .{chunk});
            std.debug.print("number! {}, i: {}\n", .{ line.number, i });

            try std.testing.expectEqual(line.number, i);
            try std.testing.expectEqual(line.line_type, LineType.Text); // Assuming all lines in this test are text
            i += 1;
        }
    }

    for (gmi.lines.items) |line| {
        std.debug.print("GMI lines? {s}\n", .{line.line});
    }

    //    try std.testing.expectEqual(lines.items.len, 3);
}

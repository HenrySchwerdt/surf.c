add_rules("mode.debug", "mode.release")
add_rules("mode.debug", "mode.release")

target("hello_world")
    set_kind("binary")
    add_headerfiles("src/*.h")
    add_files("examples/hello_world/*.c")

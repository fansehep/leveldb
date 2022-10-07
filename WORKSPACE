workspace(name = "leveldb")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    sha256 = "a1d3123179024258f9c399d45da3e0b09c4aaf8d2c041466ce5b4793a8929f23",
    strip_prefix = "googletest-86add13493e5c881d7e4ba77fb91c1f57752b3a4",
    urls = ["https://github.com/google/googletest/archive/86add13493e5c881d7e4ba77fb91c1f57752b3a4.zip"],
)

# generate compile_commands.json
http_archive(
    name = "hedron_compile_commands",

    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here. 
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    urls = [
        "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/af9af15f7bc16fc3e407e2231abfcb62907d258f.tar.gz",
    ],
    strip_prefix = "bazel-compile-commands-extractor-af9af15f7bc16fc3e407e2231abfcb62907d258f",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..." 
)
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
hedron_compile_commands_setup()

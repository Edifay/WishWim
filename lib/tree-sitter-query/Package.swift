// swift-tools-version:5.3

import PackageDescription

let package = Package(
    name: "TreeSitterQuery",
    platforms: [.macOS(.v10_13), .iOS(.v11)],
    products: [
        .library(name: "TreeSitterQuery", targets: ["TreeSitterQuery"]),
    ],
    dependencies: [],
    targets: [
        .target(name: "TreeSitterQuery",
            path: ".",
            exclude: [
                "Cargo.toml",
                "Makefile",
                "binding.gyp",
                "bindings/c",
                "bindings/go",
                "bindings/node",
                "bindings/python",
                "bindings/rust",
                "examples",
                "grammar.js",
                "package.json",
                "package-lock.json",
                "pyproject.toml",
                "setup.py",
                "test",
                "types",
                ".tests",
                ".editorconfig",
                ".github",
                ".gitignore",
                ".gitattributes",
            ],
            sources: [
                "src/parser.c",
            ],
            publicHeadersPath: "bindings/swift",
            cSettings: [.headerSearchPath("src")])
    ],
    cLanguageStandard: .c11
)

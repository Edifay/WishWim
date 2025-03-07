// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "TreeSitterMake",
    products: [
        .library(name: "TreeSitterMake", targets: ["TreeSitterMake"]),
    ],
    dependencies: [
        .package(url: "https://github.com/ChimeHQ/SwiftTreeSitter", from: "0.8.0"),
    ],
    targets: [
        .target(
            name: "TreeSitterMake",
            dependencies: [],
            path: ".",
            sources: [
                "src/parser.c",
            ],
            resources: [
                .copy("queries")
            ],
            publicHeadersPath: "bindings/swift",
            cSettings: [.headerSearchPath("src")]
        ),
        .testTarget(
            name: "TreeSitterMakeTests",
            dependencies: [
                "SwiftTreeSitter",
                "TreeSitterMake",
            ],
            path: "bindings/swift/TreeSitterMakeTests"
        )
    ],
    cLanguageStandard: .c11
)

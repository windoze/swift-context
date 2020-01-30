// swift-tools-version:5.1

import PackageDescription

let package = Package(
    name: "Context",
    products: [
        .library(
            name: "Context",
            targets: ["Context"])],
    targets: [
        .target(
            name: "Context",
            path: "Sources"),
        .testTarget(
            name: "ContextTests",
            dependencies: ["Context"])]
)

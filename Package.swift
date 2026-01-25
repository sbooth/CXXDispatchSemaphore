// swift-tools-version: 5.9
//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXDispatchSemaphore
//

import PackageDescription

let package = Package(
    name: "CXXDispatchSemaphore",
    products: [
        .library(
            name: "CXXDispatchSemaphore",
            targets: [
                "CXXDispatchSemaphore",
            ]
        ),
    ],
    targets: [
        .target(
            name: "CXXDispatchSemaphore",
            cxxSettings: [
                .headerSearchPath("include/CXXDispatchSemaphore"),
            ]
        ),
        .testTarget(
            name: "CXXDispatchSemaphoreTests",
            dependencies: [
                "CXXDispatchSemaphore",
            ],
            swiftSettings: [
                .interoperabilityMode(.Cxx),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx17
)

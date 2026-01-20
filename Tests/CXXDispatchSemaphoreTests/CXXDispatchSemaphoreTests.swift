//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXDispatchSemaphore
//

import Testing
import Foundation
@testable import CXXDispatchSemaphore

@Suite struct CXXDispatchSemaphoreTests {
    @Test func basic() async {
        var sema = cxxdsema.DispatchSemaphore(0)
        let didAcquire = sema.wait(DispatchTime.now().rawValue)
        #expect(!didAcquire)
    }

    @Test func wrapped() async {
        let dsema = DispatchSemaphore(value: 0)
        var sema = cxxdsema.DispatchSemaphore(dsema)
        let didAcquire = sema.wait(DispatchTime.now().rawValue)
        #expect(!didAcquire)
    }
}

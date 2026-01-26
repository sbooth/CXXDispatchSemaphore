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
        var semaphore = dsema.Semaphore(0)
        let didAcquire = semaphore.wait(DispatchTime.now().rawValue)
        #expect(!didAcquire)
    }

    @Test func wrapped() async {
        let dispatchSemaphore = DispatchSemaphore(value: 0)
        var semaphore = dsema.Semaphore(dispatchSemaphore)
        let didAcquire = semaphore.wait(DispatchTime.now().rawValue)
        #expect(!didAcquire)
    }
}

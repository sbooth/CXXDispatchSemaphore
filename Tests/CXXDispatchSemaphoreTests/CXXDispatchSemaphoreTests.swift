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
		var sema = CXXDispatchSemaphore.DispatchSemaphore(0)
		let decremented = sema.wait(DispatchTime.now().rawValue)
		#expect(!decremented)
	}
}

import XCTest
@testable import Context

class ContextTests: XCTestCase {
    func testExample() {
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
        //XCTAssertEqual(Context().text, "Hello, World!")
    }


    static var allTests : [(String, (ContextTests) -> () throws -> Void)] {
        return [
            ("testExample", testExample),
        ]
    }
}

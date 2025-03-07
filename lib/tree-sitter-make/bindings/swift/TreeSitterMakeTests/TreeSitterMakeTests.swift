import XCTest
import SwiftTreeSitter
import TreeSitterMake

final class TreeSitterMakeTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_make())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Make grammar")
    }
}

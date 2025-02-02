import XCTest
import SwiftTreeSitter
import TreeSitterVhdl

final class TreeSitterVhdlTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_vhdl())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Vhdl grammar")
    }
}

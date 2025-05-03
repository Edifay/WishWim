import XCTest
import SwiftTreeSitter
import TreeSitterAsm

final class TreeSitterAsmTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_asm())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Asm grammar")
    }
}

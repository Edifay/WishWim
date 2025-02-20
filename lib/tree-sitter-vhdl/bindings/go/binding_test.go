package tree_sitter_vhdl_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_vhdl "github.com/tree-sitter/tree-sitter-vhdl/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_vhdl.Language())
	if language == nil {
		t.Errorf("Error loading Vhdl grammar")
	}
}

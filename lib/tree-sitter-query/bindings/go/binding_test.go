package tree_sitter_query_test

import (
	"testing"

	tree_sitter "github.com/smacker/go-tree-sitter"
	"github.com/tree-sitter-grammars/tree-sitter-query"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_query.Language())
	if language == nil {
		t.Errorf("Error loading Query grammar")
	}
}

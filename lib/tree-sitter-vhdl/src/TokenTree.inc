static TypeNode* type_node_new(TokenType type)
{
    TypeNode* this = ts_malloc(sizeof(TypeNode));
    this->type = type;
    this->next = 0;
    return this;
}
//------------------------------------------------------------------------------

static void type_node_free(TypeNode* this)
{
    if(this->next) type_node_free(this->next);
    ts_free(this);
}
//------------------------------------------------------------------------------

static TypeNode* type_node_insert(TypeNode* head, TokenType type)
{
    TypeNode* temp = head;
    while(temp){
        if(temp->type == type){
            warning("Duplicate entry for type %d (%s)", type, token_type_to_string(type));
            return head;
        }
        temp = temp->next;
    }

    temp = type_node_new(type);
    temp->next = head;
    return temp;
}
//------------------------------------------------------------------------------

typedef struct NodeTag{
    uint32_t  character; // The character at the current depth
    TypeNode* type;      // Linked list of types

    struct NodeTag* left;
    struct NodeTag* right;

    struct NodeTag* next; // Sub-tree of the next character
} Node;
static Node* node_new (char character);
static void  node_free(Node* this);
//------------------------------------------------------------------------------

static Node* insert(Node* root, const char* pattern, TokenType type);

// Balancing functions
static Node* balance    (Node* root);
static Node* compress   (Node* root, int count);
static void  sub_balance(Node* node);
//------------------------------------------------------------------------------

static Node* node_new(char character)
{
    Node* this = ts_malloc(sizeof(Node));
    if(!this) return 0;

    this->left = this->right = this->next = 0;

    this->character = (unsigned)character;
    this->type      = 0;

    return this;
}
//------------------------------------------------------------------------------

static void node_free(Node* this)
{
    if(this->next)  node_free     (this->next);
    if(this->left)  node_free     (this->left);
    if(this->right) node_free     (this->right);
    if(this->type)  type_node_free(this->type);
    ts_free(this);
}
//------------------------------------------------------------------------------

TokenTree* token_tree_new()
{
    TokenTree* this = ts_malloc(sizeof(TokenTree));
    if(!this) return 0;

    this->root = 0;
    return this;
}
//------------------------------------------------------------------------------

void token_tree_free(TokenTree* this)
{
    if(this->root) node_free(this->root);
    ts_free(this);
}
//------------------------------------------------------------------------------

void token_tree_insert(TokenTree* this, const char* pattern, TokenType type)
{
    this->root = insert(this->root, pattern, type);
}
//------------------------------------------------------------------------------

static void node_setup(Node* node, const char* pattern, TokenType type)
{
    if(pattern[0] == '_'){
        if(!node->type) node->type = type_node_insert(node->type, IDENTIFIER_EXPECTING_LETTER);
    }
    if(pattern[1]) node->next = insert(node->next, pattern+1, type);
    else           node->type = type_node_insert(node->type, type);
}
//------------------------------------------------------------------------------

static Node* insert(
    Node*       root,
    const char* pattern,
    TokenType   type
){
    if(!pattern[0]) return root;

    // Keep in vine structure until balancing restructures the tree
    Node* node;
    Node* prev = 0;
    Node* temp = root;

    while(temp){
        if((uint32_t)(*pattern) < temp->character){
            node        = node_new(*pattern);
            node->right = temp;
            node_setup(node, pattern, type);
            if(prev) prev->right = node;
            else     root        = node;
            return root;

        }else if((uint32_t)(*pattern) > temp->character){
            prev = temp;
            temp = temp->right;

        }else{
            node_setup(temp, pattern, type);
            return root;
        }
    }
    node = node_new(*pattern);
    node_setup(node, pattern, type);
    if(prev) prev->right = node;
    else     root        = node;

    return root;
}
//------------------------------------------------------------------------------

/** This balancing operation is based on:
    Quentin F Stout and Bette L Warren,
    "Tree rebalancing in optimal space and time"
    Communications of the ACM, September 1986, Volume 29, Number 9
    https://web.eecs.umich.edu/~qstout/pap/CACM86.pdf
    https://deepblue.lib.umich.edu/bitstream/handle/2027.42/7801/bad3920.0001.001.pdf?sequence=5&isAllowed=y */

void token_tree_balance(TokenTree* this)
{
    this->root = balance(this->root);
}
//------------------------------------------------------------------------------

static Node* balance(Node* root){
    if(!root) return 0;

    // count the items in the vine
    int   count = 0;
    Node* node  = root;
    while(node){
        count++;
        node = node->right;
    }

    // Create the deepest leaves
    int n    = 0x8000;
    int size = count + 1;
    while(n > size) n >>= 1; // n = 2^floor(log2(count + 1))
    size -= n;

    if(size) root = compress(root, size);
    size = count - size;

    // Balance the tree
    while(size > 1){
        size /= 2;
        root  = compress(root, size);
    }

    sub_balance(root);

    return root;
}
//------------------------------------------------------------------------------

static Node* compress(Node* root, int count)
{
    Node* temp  = root->right;
    root->right = temp->left;
    temp->left  = root;
    root        = temp;

    int   n;
    Node* node = root;

    for(n = 1; n < count; n++){
        temp               = node->right->right;
        node->right->right = temp->left;
        temp->left         = node->right;
        node->right        = temp;
        node               = temp;
    }
    return root;
}
//------------------------------------------------------------------------------

static void sub_balance(Node* node)
{
    if(node->next) node->next = balance(node->next);

    if(node->left ) sub_balance(node->left );
    if(node->right) sub_balance(node->right);
}
//------------------------------------------------------------------------------

static int32_t lowercase(int32_t lookahead)
{
    if(lookahead >= 'A' && lookahead <= 'Z') lookahead += 'a' - 'A';
    debug("lookahead = %c(0x%x)", (char)lookahead, lookahead);
    return lookahead;
}
//------------------------------------------------------------------------------

static int32_t advance(TSLexer* lexer)
{
    lexer->advance(lexer, false);
    return lowercase(lexer->lookahead);
}
//------------------------------------------------------------------------------

TypeNode* token_tree_match(TokenTree* this, TSLexer* lexer)
{
    uint32_t  lookahead = lowercase(lexer->lookahead);
    TypeNode* type      = 0;
    Node*     node      = this->root;

    while(node && !lexer->eof(lexer)){
        if(lookahead < node->character){
            node = node->left;

        }else if(lookahead > node->character){
            node = node->right;

        }else{
            lookahead = advance(lexer);
            if(node->type) lexer->mark_end(lexer);
            type = node->type;
            node = node->next;
        }
    }
    debug("type->head = %s", type ? token_type_to_string(type->type) : "UNKNOWN");
    return type;
}
//------------------------------------------------------------------------------


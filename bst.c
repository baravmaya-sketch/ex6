#include <stdlib.h>
#include "bst.h"

BST* createBST(int (*cmp)(void*, void*), void (*print)(void*), void (*freeData)(void*)) {

    BST* newBST = malloc(sizeof(BST));
    if (!newBST) exit(1);

    newBST->root = NULL;
    newBST->compare = cmp;
    newBST->freeData = freeData;
    newBST->print = print;

    return newBST;
}

//main function to create a Node
BSTNode* bstInsert(BSTNode* root, void* data, int (*cmp)(void*, void*)) {
    if (root == NULL)
        return createNode(data);

    if (cmp(data, root->data) < 0)
        root->left = bstInsert(root->left, data, cmp);
    else
        root->right = bstInsert(root->right, data, cmp);

    return root;
}

//helper function to add a node
static BSTNode* createNode(void* data) {
    BSTNode* newNode = (BSTNode*)malloc(sizeof(BSTNode));
    if (newNode == NULL) {
        exit(1);
    }
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

//realses tree memory safely
void bstFree(BSTNode* root, void (*freeData)(void*)) {
    //stop condition
    if (root == NULL)
        return;

    //recursion steps to bith sides
    bstFree(root->right, freeData);
    bstFree(root->left, freeData);

    //to be on safe side - if func exists
    if (freeData != NULL)
        freeData(root->data);

    free(root);
}

//prints in order
void bstInorder(BSTNode* root, void (*print)(void*)) {
    if (!root)
        return;
 
    bstInorder(root->left, print);
    
    if (print)
        print(root->data);
    
    bstInorder(root->right, print);          
}

//prints preorder
void bstPreorder(BSTNode* root, void (*print)(void*)) {
    if (!root)
        return;

    if (print)
        print(root->data);

    bstPreorder(root->left, print);
    bstPreorder(root->right, print);
}

//prints post order
void bstPostorder(BSTNode* root, void (*print)(void*)) {
    if (!root)
        return;

    bstPostorder(root->left, print);
    bstPostorder(root->right, print);
    if (print)
        print(root->data);
}

//search for node equal data
void* bstFind(BSTNode* root, void* data, int (*cmp)(void*, void*)) {
    BSTNode* iterNode = root;

    while (iterNode) {
        int res = cmp(data, iterNode->data);
        if (res == 0)
            return iterNode->data;
        iterNode = (res < 0) ? iterNode->left : iterNode->right;
    }

    return NULL;
}
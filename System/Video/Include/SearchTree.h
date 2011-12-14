#ifndef SEARCHTREE_H_
#define SEARCHTREE_H_

typedef struct numNode {
		int num;
		struct numNode *left;
		struct numNode *right;
} NumNode;

bool LeftRotate(NumNode *node){
	NumNode *moveNode;
	int num;
	if(node == NULL || node->right == NULL)
		return false;

	moveNode = node->right;
	node->right = moveNode->right;
	moveNode->right = moveNode->left;
	moveNode->left = node->left;
	node->left = moveNode;

	num = node->num;
	node->num = moveNode->num;
	moveNode->num = num;
	return true;
}

bool RightRotate(NumNode *node) {
	NumNode *moveNode;
	int num;
	if(node == NULL || node->left == NULL)
		return false;

	moveNode = node->left;
	node->left = moveNode->left;
	moveNode->left = moveNode->right;
	moveNode->right = node->right;
	node->right = moveNode;

	num = node->num;
	node->num = moveNode->num;
	moveNode->num = num;
	return true;
}

void BalanceTree(NumNode *root) {
	NumNode *p;
	int nodecount;
	int i;

	for(p=root,nodecount=0; p!=0; p=p->right,++nodecount)
		while(RightRotate(p)) { };

	for(i=nodecount/2; i>0; i/=2 ) {
		int k;
		for(p=root,k=0;k<i;++k,p=p->right)
			LeftRotate(p);
	}
}

int TreeHeight(NumNode *root) {
	if(root == NULL)
		return 0;
	int hLeft = TreeHeight(root->left);
	int hRight = TreeHeight(root->right);
	return 1 + ((hLeft >= hRight) ? hLeft : hRight);
}

bool InsertValue(NumNode **root,int value) {
	NumNode *p;
	NumNode *nw;
	if((nw=(NumNode*)malloc(sizeof(NumNode))) == NULL)
		return false;

	nw->num 	= value;
	nw->left 	= NULL;
	nw->right 	= NULL;

	if(*root==NULL) {
		(*root)=nw;
		return true;
	}

	for(p=*root; ; ) {
		if(value < p->num) {
			if(p->left == NULL) {
				p->left = nw;
				return true;
			}
			p=p->left;
		}
		else if(value > p->num) {
			if(p->right == NULL) {
				p->right = nw;
				return true;
			}
			p = p->right;
		}
		else {
			free(nw);
			return false;
		}
	}
}

void PrintTree(NumNode *root) {
	if(root == NULL)
		return;
	PrintTree(root->left);
	printf(" %d ",root->num);
	PrintTree(root->right);
}

void DeleteTree(NumNode *root) {
	if(root==0)
		return;
	DeleteTree(root->left);
	DeleteTree(root->right);
	free(root);
}

bool SearchTree(NumNode *root, int key) {
	if(root == 0)
		return false;
	else if(key == root->num)
		return true;
	else if(key < root->num)
		return SearchTree(root->left,key);
	else if(key > root->num)
		return SearchTree(root->right,key);
}

#endif /* SEARCHTREE_H_ */

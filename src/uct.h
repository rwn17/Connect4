#ifndef UCT_H_
#define UCT_H_

#include "Point.h"
#include <iostream>

#define MAX_M 12
#define MAX_N 12

const double Lambda = 0.5;
const int max_search_number = 5000000;
const int max_size = 5000000;
const int time_bound = 2.5;


//next player

inline int compute_next_player(int player)
{
	if (player == 1)
		return 2;
	else return 1;
}

// fixed lenth vector 
template<typename T,int n>
class MyVector{

private:
	T _data[n];
	int _size;
public:
	bool empty()  { return  _size == 0 ; }

	void clear () {  _size = 0 ; }
	
	void push_back ( const T &t) {  _data[_size++] = t ;}

	unsigned size() { return _size ; }

	T &operator[] (unsigned i) { return _data[i];}

	T pop_back()
	{
		T t = _data[_size-1];
		_size -= 1;
		return t;
	}
	
	void remove( T data)
	{
		int i = 0;
		for ( i = 0 ; i < _size ; ++i)
		{
			if (_data[i] == data)
				break;
		}
		--_size;
		for ( ; i < _size ; ++i)
		{
			_data[i] = _data[i+1];
		}
	}
};

//node for uct tree
struct Node{
		Point action;
		Node *parent;
		int player;
		double Q;
		int N;
		bool full;
		MyVector< int, MAX_N> child_action;
		MyVector< Node *, MAX_N> child_node;

		Node() : action( 0 , 0 ) , parent( nullptr ), player (-1) ,Q(0), N(0),full(0) {}

		int next_player ();

		double up_bound(double _log);
};

//factory for node
class Factory {
public:
	Factory():space(new Node[max_size + 1]), total_size(max_size + 1), top(0){}

    ~Factory(){delete[]space;}

    Node *add_node(Node *parent , unsigned player, Point action);

    bool empty() { return top < total_size; }

    void clear() { top = 0; }

private:
    Node *space;
    int total_size;
    int top;
};


class UCT {
public: 
	Point uct_search();

	void init(int M, int N, int **board, const int *top, int noX, int noY, int player );

	void print_board();

private:
	Factory factory;
	int **_init_board, **_crt_board;
	int _player, _winner;
	int _noX , _noY, _M , _N;
	int _total_num;
	double _total_log;
	int *_init_top, *_crt_top;

	Node * init_node();

	Node *tree_policy( Node *v);

	void add_action(Node *v);

	void take_action( const Point &action, int player);

	Node *expand( Node *v);

	Node *best_child( Node *v);

	double default_policy(Node *v, int player);
	
	void back_up( Node *v, double reward);

	void clear();
};

//test whether user/machine win
bool ifWin(const int x, const int y, const int M, const int N, int* const* board, int player)
{
	if (player ==1 && userWin(x,y,M,N,board)==1) {  return 1; }
	else if ( player ==2 && machineWin(x,y,M,N,board)==1) {return 1;}
	else {return 0;}
}

inline double time_diff(const timeval &t1, const timeval &t2)
{
    return t1.tv_sec - t2.tv_sec + static_cast<double>(t1.tv_usec - t2.tv_usec) / 1000000;
}

inline int Node::next_player ()
{
	return compute_next_player( player);
}


inline double Node::up_bound(double _log)
{
    return Q / N +  Lambda * _log / sqrt(N);
}

#endif
#include "Point.h"
#include "Judge.h"
#include "uct.h"
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>


Node *Factory::add_node(Node *parent, unsigned player, Point action)
{
	Node *v = space + top;
	if (top == total_size)
	{
		std::cout<<"BOOM!!!!!!\n";
		v->full = 1;
	}
	top += 1;
	v->N = 0;
	v->Q = 0;
	v->parent = parent;
	v->player = player;
	v->action = action;
	v->child_node.clear();
	v->child_action.clear();
	return v;
}


void UCT::init(int M, int N, int **board, const int *top, int noX, int noY, int player )
{
	_M = M;
	_N = N;
	_noX = noX;
	_noY = noY;
	_player = player;
	_winner = -1;

	//board init
	_init_board = new int *[M];
	_crt_board = new int *[M];
	for (int  i = 0; i< M; ++i)
	{
		_init_board[i] = new int[N];
		_crt_board[i] = new int[N];
	}
	_init_top = new int [M];
	_crt_top = new int [M];
	for ( int i  = 0; i < N ; ++i)
	{
		for (int j = 0; j < M ; ++j)
		{
			_init_board[j][i] = _crt_board[j][i]= board[j][i];
		}
		_init_top [i] = _crt_top[i] = top[i];
	}
}

Point UCT::uct_search()
{
	std::cout<<"search start"<<std::endl;
	srand(time(nullptr));
	int search_num = 0;
	timeval begin_time, crt_time;
	struct timezone zone;
	gettimeofday(&begin_time,&zone);
	Node *v0 = init_node();
	while (search_num < max_search_number)
	{
		if ((search_num % 1000) == 0)
		{
			gettimeofday(&crt_time,&zone);
			if ( time_diff(crt_time,begin_time) > time_bound)
			{
				break;
			}
				
		}
		Node *v1 = tree_policy(v0);
		double reward  = default_policy(v1,v1->player);
		back_up(v1 , reward);
		clear();
		search_num ++;
	}
	return best_child(v0)->action;
}

Node * UCT::init_node()
{
	Point init_point(-1,-1);
	Node *v = factory.add_node(nullptr, _player, init_point );
	v->player = compute_next_player(_player);
	add_action(v);
	return v;
}

void UCT::add_action( Node *v)
{
	for ( int i = 0; i < _N; ++i)
	{
		if (_crt_top[i] > 0)
		{
			v->child_action.push_back(i);
		}
	}

}

 Node *UCT::expand(Node *v)
 {
	 Node *child= nullptr;
	 int action = v->child_action.pop_back();
	 Point p1(_crt_top[action]-1,action);
	 child = factory.add_node( v , v->next_player(), p1);
	 v->child_node.push_back(child);
	 take_action(p1,child->player);
	 if (_winner == -1)
	 {
	 add_action(child);
	 }
	 return child;
 }

 Node *UCT::tree_policy(Node *v)
 {
	 do
	 {
		 if ( !v->child_action.empty() ) {   return expand(v); }
		 else {
			v = best_child(v);
			take_action ( v->action,v->player);
		 }
	 }while (_winner == -1);
	 return v;
 }


 void UCT::take_action( const Point &action, int player)
 {
	_crt_board[action.x][action.y] = player;
	_crt_top[action.y] -= 1;
	if (action.y == _noY && action.x == _noX + 1)
		 _crt_top [action.y] -= 1;
	if (ifWin(action.x, action.y, _M,_N,_crt_board,player))
	{
		 _winner = player;
	} else if (isTie(_N,_crt_top))
	{
		 _winner = 0;
	}
 }

 double UCT::default_policy( Node *v, int player)
 {
	 int this_player = compute_next_player(player);
	 int feasible_actions[MAX_N];
	 int feasible_num = 0;
	 int choice;
	 int chosen_y;
	 for (int i = 0; i < _N; ++i)
     {
		 if (_crt_top[i] > 0)  { feasible_actions[feasible_num++]  = i ;  }
	  }
	 while( _winner == -1)
	 {
		choice = rand() % feasible_num;
		chosen_y = feasible_actions [ choice ] ;
		Point action( _crt_top [ chosen_y ] - 1, chosen_y);
		take_action( action , this_player );
		if (_crt_top[chosen_y] == 0)
		{
            feasible_num--;
            for (int i = choice; i < feasible_num; ++i)
                feasible_actions[i] = feasible_actions[i + 1];
		}
		this_player = compute_next_player(this_player);
	 } 

    if (_winner == player )
        return 1;
    else if ( _winner == 0 )
        return 0;
    else
        return -1;
}

 Node *UCT::best_child(Node *v)
 {
	 double max_ucb = -1e10;
	 Node * max_node  = nullptr;
	 for( unsigned int i = 0 ; i < v->child_node.size() ; ++i)
	 {

		 double ucb = v->child_node[i]->up_bound(_total_log);
		 if  (ucb > max_ucb)
		 {
			 max_ucb = ucb;
			 max_node = v->child_node[i];
		 }
	 }
	 if (max_node == nullptr) { max_node = v->child_node[rand() % v->child_node.size()]; }
	 return max_node;
 }

 void UCT::back_up( Node *v, double reward)
 {
	 while ( v != nullptr)
	 {
		 v->N += 1;
		 v->Q += reward;
		 reward = -reward;
		 v = v->parent;
	 }
	 _total_num += 1;
	 _total_log = sqrt(log(_total_num));
 }

 void UCT::clear()
 {
	 for ( int i = 0 ; i < _N ; ++i)
	 {
		 for ( int j = 0 ; j < _M ; ++j)
		 {
			 _crt_board [j][i] = _init_board[j][i];
		 }
		 _crt_top [i] = _init_top[i];
	 }
	 _winner = -1;
 }

 void UCT::print_board()
 {
	 for (int i = 0; i < _M; ++i)
    {
        for (int j = 0; j < _N; ++j)
            if (i == _noX && j == _noY)
                std::cout << "x ";
            else
                std::cout << _crt_board[i][j] << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
 }


/* for debug
int main()
{	
	int M = 11;
	int N = 12;
	int noX = 5;
	int noY = 5;
	int **board = new int *[M];
	for (int i = 0; i < M; i++)
	{
		board[i] = new int[N];
	}
	int *top = new int[N];
	for ( int i = 0; i < N ; ++i )
	{
		top[i] = M;
	}
	top [3] =M-1;
	board[M-1][3] = 2;
	top [6] =M-1;
	board[M-1][6] = 1;
	UCT uct_tree;
    uct_tree.init(M, N, board, top, noX, noY, 1);
	uct_tree.print_board();
    Point action = uct_tree.uct_search();
    int x = action.x;
    int y = action.y;
	std::cout<<x<<"  "<<y<<std::endl;
}



bool userWin(const int x, const int y, const int M, const int N, int *const *board)
{
    //横向检测
    int i, j;
    int count = 0;
    for (i = y; i >= 0; i--)
        if (!(board[x][i] == 1))
            break;
    count += (y - i);
    for (i = y; i < N; i++)
        if (!(board[x][i] == 1))
            break;
    count += (i - y - 1);
    if (count >= 4)
        return true;

    //纵向检测
    count = 0;
    for (i = x; i < M; i++)
        if (!(board[i][y] == 1))
            break;
    count += (i - x);
    if (count >= 4)
        return true;

    //左下-右上
    count = 0;
    for (i = x, j = y; i < M && j >= 0; i++, j--)
        if (!(board[i][j] == 1))
            break;
    count += (y - j);
    for (i = x, j = y; i >= 0 && j < N; i--, j++)
        if (!(board[i][j] == 1))
            break;
    count += (j - y - 1);
    if (count >= 4)
        return true;

    //左上-右下
    count = 0;
    for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
        if (!(board[i][j] == 1))
            break;
    count += (y - j);
    for (i = x, j = y; i < M && j < N; i++, j++)
        if (!(board[i][j] == 1))
            break;
    count += (j - y - 1);
    if (count >= 4)
        return true;

    return false;
}

bool machineWin(const int x, const int y, const int M, const int N, int *const *board)
{
    //横向检测
    int i, j;
    int count = 0;
    for (i = y; i >= 0; i--)
        if (!(board[x][i] == 2))
            break;
    count += (y - i);
    for (i = y; i < N; i++)
        if (!(board[x][i] == 2))
            break;
    count += (i - y - 1);
    if (count >= 4)
        return true;

    //纵向检测
    count = 0;
    for (i = x; i < M; i++)
        if (!(board[i][y] == 2))
            break;
    count += (i - x);
    if (count >= 4)
        return true;

    //左下-右上
    count = 0;
    for (i = x, j = y; i < M && j >= 0; i++, j--)
        if (!(board[i][j] == 2))
            break;
    count += (y - j);
    for (i = x, j = y; i >= 0 && j < N; i--, j++)
        if (!(board[i][j] == 2))
            break;
    count += (j - y - 1);
    if (count >= 4)
        return true;

    //左上-右下
    count = 0;
    for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
        if (!(board[i][j] == 2))
            break;
    count += (y - j);
    for (i = x, j = y; i < M && j < N; i++, j++)
        if (!(board[i][j] == 2))
            break;
    count += (j - y - 1);
    if (count >= 4)
        return true;

    return false;
}

bool isTie(const int N, const int *top)
{
    bool tie = true;
    for (int i = 0; i < N; i++)
    {
        if (top[i] > 0)
        {
            tie = false;
            break;
        }
    }
    return tie;
}
*/

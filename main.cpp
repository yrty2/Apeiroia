#include<iostream>
#include<thread>
#include<string>
#include<chrono>
#include<array>
#include<random>
#include<ncurses.h>
constexpr int width=64;
constexpr int height=48;
const std::string wall="#";
const std::string unpathable="W";
const std::string playerch="@";

std::array<std::array<bool,height>,width> generate(){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0,2);
	std::uniform_int_distribution<> dir(0,3);
	//棒倒し法アルゴリズム
	std::array<std::array<bool,height>,width> res{};
	//initialization
	for(int i=0; i<width; ++i){
		for(int j=0; j<height; ++j){
			res[i][j]=i%2==1 && j%2==1;
		}
	}
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			if(i%2==1 && j%2==1){
				int seed;
				if(i==1){
					seed=dir(gen);
				}else{
					seed=dist(gen);
				}
				//construction
				//vector
				int x=0;
				int y=0;
				if(seed==0){
					//left
					x=-1;
					y=0;
				}
				if(seed==1){
					//right
					x=1;
					y=0;
				}
				if(seed==2){
					//down
					x=0;
					y=1;
				}
				if(seed==3){
					//up
					x=0;
					y=-1;
				}
				if(i+y<height && j+x<width){
					res[j+x][i+y]=1;
				}
			}
		}
	}
	return res;
}
int main(){
	initscr();
	cbreak();
	noecho();
	int u[2]={0,0};
const int FPS=60;
const std::chrono::milliseconds frameTime(1000/FPS);
std::array<std::array<bool,height>,width> map=generate();
while(true){
	clear();
	std::string ctx="The random map\n";
	for(int k=0; k<width+2; ++k){
		ctx+=unpathable+" ";
	}
	ctx+="\n";
	for(int i=0; i<height; ++i){
		ctx+=unpathable+" ";
		for(int j=0; j<width; ++j){
			if(map[j][i]){
				ctx+=wall+" ";
			}else{
				if(i==u[1] && j==u[0]){
					ctx+=playerch+" ";
				}else{
					ctx+="  ";
				}
			}
		}
		ctx+=unpathable+"\n";
	}
	for(int k=0; k<width+2; ++k){
		ctx+=unpathable+" ";
	}
	printw("%s",ctx.c_str());
	refresh();
	//keyevent
	std::string c=std::string(1,getch());
	if(c=="w"){
		if(u[1]-1>=0 && !map[u[0]][u[1]-1]){
			u[1]--;
		}
	}
	if(c=="a"){
		if(u[0]-1>=0 && !map[u[0]-1][u[1]]){
			u[0]--;
		}
	}
	if(c=="s"){
		if(u[1]+1<height && !map[u[0]][u[1]+1]){
			u[1]++;
		}
	}
	if(c=="d"){
		if(u[0]+1<width && !map[u[0]+1][u[1]]){
			u[0]++;
		}
	}
}
endwin();
}

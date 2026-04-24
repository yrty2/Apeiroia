#include<iostream>
#include<string>
#include<array>
#include<random>
#include<ncurses.h>
#include<vector>
#include<cmath>

using namespace std;

constexpr int width=78;//should be %2==0
constexpr int height=50;
constexpr int sight=4;
constexpr int wallheight=50;
constexpr double pi=3.14159265358979323846;
constexpr double fp=0.6;//視力が悪いとこの値が大きい？
constexpr double viewwidth=0.5;
const string wall="#";
const string unpathable="W";
const string playerch="@";

template<typename S>
using Space=array<array<S,height>,width>;
using vec2=array<int,2>;
using complex=array<double,2>;

int mod(int a,int b){
	/*if(a>0){
		return a%b;
	}
	if(a%b==0){
		return 0;
	}
	return a%b+b;*/
	return (a%b+b)%b;
}
double mod(double a,double b){
	return a-b*floor(a/b);
}
class Tile{
	public:
		int weight=255;
		int position[2]={0,0};
		bool wall;
		bool revealed=false;
		bool view=false;
		bool pathed=false;
		array<bool,4> open={};
		Tile(){
			wall=false;
			open={0,0,0,0};
		}
        Tile(bool W,array<bool,4> direction,int i,int j){
			wall=W;
			open=direction;
			position[0]=i;
			position[1]=j;
		}
    //private:
};
Tile touch(Space<Tile> map,int u[2],int dx,int dy){
	return map[mod(u[0]+dx,width)][mod(u[1]+dy,height)];
}
Tile invtouch(Space<Tile> map,int u[2]){
	return map[mod(u[0],height)][mod(u[1],width)];
}
void randomize(Space<Tile>& map, int u[2]){
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> randw(0,width-1);
	uniform_int_distribution<> randh(0,height-1);
	int i=-1;
	int j=-1;
	while(true){
		i=randw(gen);
		j=randh(gen);
		if(!map[i][j].wall){	
			break;
		}
	}
	u[0]=i;
	u[1]=j;
}
bool ismovable(Space<Tile>& map,int u[2],int dx,int dy){
	return !map[mod(u[0]+dx,width)][mod(u[1]+dy,height)].wall;	
}
Space<Tile> generate(){
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(0,2);
	uniform_int_distribution<> dir(0,3);
	//棒倒し法アルゴリズム
	Space<Tile> res={};
	//initialization
	for(int i=0; i<width; ++i){
		for(int j=0; j<height; ++j){
			res[i][j]=Tile(i%2==1 && j%2==1,{0,0,0,0},i,j);
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
					res[j+x][i+y].wall=1;
				}
			}
		}
	}
	//room生成(ランダム)
	uniform_int_distribution<> rooma(0,20);
	uniform_int_distribution<> randw(0,width-1);
	uniform_int_distribution<> randh(0,height-1);

	int amount=rooma(gen);
	vector<vec2> rooms={};
		for(int k=0; k<amount; ++k){
			vec2 u={0,0};
			while(true){
				u[0]=randw(gen);
				u[1]=randh(gen);
				break;
			}
			rooms.push_back(u);
		for(int i=0; i<7; ++i){
			for(int j=0; j<5; ++j){
				res[i+u[0]][j+u[1]].wall=0;
			}
		}
	}
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			if(!res[j][i].wall){
				if(ismovable(res,res[j][i].position,0,-1)){
					res[j][i].open[0]=1;
				}
				if(ismovable(res,res[j][i].position,-1,0)){
					res[j][i].open[1]=1;
				}
				if(ismovable(res,res[j][i].position,0,1)){
					res[j][i].open[2]=1;
				}
				if(ismovable(res,res[j][i].position,1,0)){
					res[j][i].open[3]=1;
				}
			}
		}
	}
	return res;
}
vec2 i2v(int i){
	vec2 res={0,0};
	if(i==0){
		res[1]--;
	}
	if(i==1){
		res[0]--;
	}
	if(i==2){
		res[1]++;
	}
	if(i==3){
		res[0]++;
	}
	return res;
}
vector<Tile> bfs(Space<Tile>& map,int pos[2],int far,bool addwall){
	Tile origin=map[pos[0]][pos[1]];
	vector<Tile> look={origin};
	vector<Tile> res={};
	vector<Tile> looked={};
	int weigh=0;
	while(true){
		for(int k=0; k<look.size(); ++k){
			int tp[2]={look[k].position[0],look[k].position[1]};
			look[k].weight=weigh;
			look[k].pathed=true;
			res.push_back(look[k]);
			map[tp[0]][tp[1]].weight=weigh;
			map[tp[0]][tp[1]].pathed=true;
			for(int i=0; i<4; ++i){
				//方向を見る
				if(look[k].open[i]){
					//iの向きが開いている
					Tile t=touch(map,tp,i2v(i)[0],i2v(i)[1]);
					if(!t.pathed){
						looked.push_back(t);
					}
				}else if(weigh<far){
					Tile t=touch(map,tp,i2v(i)[0],i2v(i)[1]);
					if(t.weight>weigh+1){
						t.weight=weigh+1;
						res.push_back(t);
					}
				}
			}
		}
		look=looked;
		looked.clear();
		weigh++;
		if(weigh>far){
			break;
		}
	}
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			map[j][i].pathed=false;
		}
	}
	return res;
}
void view(Space<Tile>& map,int u[2]){
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			map[j][i].weight=255;
			map[j][i].view=false;
		}
	}
	vector<Tile> t=bfs(map,u,sight,true);
	for(int k=0; k<t.size(); ++k){
		map[t[k].position[0]][t[k].position[1]].weight=t[k].weight;
		map[t[k].position[0]][t[k].position[1]].revealed=true;
		map[t[k].position[0]][t[k].position[1]].view=true;
	}
}
void moved(Space<Tile>& map,int u[2]){
	view(map,u);
}
complex mul(complex a,complex b){
	array<double,2> c={a[0]*b[0]-a[1]*b[1],a[0]*b[1]+a[1]*b[0]};
	return c;
}
complex sum(complex a,complex b){
	array<double,2> c={a[0]+b[0],a[1]+b[1]};
	return c;
}
complex sumi(complex a,int b[2]){
	array<double,2> c={a[0]+b[0],a[1]+b[1]};
	return c;
}
complex sub(complex a,complex b){
	array<double,2> c={a[0]-b[0],a[1]-b[1]};
	return c;
}
complex conj(complex a){
	array<double,2> c={a[0],-a[1]};
	return c;
}
complex smul(complex a,double b){
	array<double,2> c={a[0]*b,a[1]*b};
	return c;
}
complex poler(double a,double b){
	array<double,2> c={a*cos(b),a*sin(b)};
	return c;
}
complex sdiv(complex a,double b){
	array<double,2> c={a[0]/b,a[1]/b};
	return c;
}
complex neg(complex a){
	array<double,2> c={-a[0],-a[1]};
	return c;
}
double norm(complex a){
	return a[0]*a[0]+a[1]*a[1];
}
double arg(complex a){
	return atan2(a[1],a[0]);
}
complex div(complex a,complex b){
	return sdiv(mul(a,conj(b)),norm(b));
}
complex normalize(complex a){
	return sdiv(a,sqrt(norm(a)));
}
void draw(int i,int j,string str,int u[2],int color){
		if(color!=-1){
			attron(COLOR_PAIR(color));
		}
		for(int k=-1; k<=1; ++k){
			for(int h=-1; h<=1; ++h){
				vec2 p={j-u[1]+height*k+height/2,(i-u[0])+width*h+width/2};
				if(p[0]<height && p[1]<width && p[0]>=0 && p[1]>=0){
					mvprintw(p[0],2*p[1],"%s ",str.c_str());
				}
			}
		}
		if(color!=-1){
			attroff(COLOR_PAIR(color));
		}
}
void drawWithDepth(Space<Tile>& map,array<vec2,width> room,array<double,width> depth,array<int,width> wd){
	for(int k=0; k<width; ++k){
		if(depth[k]!=-1){
			int roomid[2]={room[k][0],room[k][1]};
			//見ているものは正しいようだ。
			Tile t=touch(map,roomid,0,0);
			
				double yh=wallheight/(1+depth[k]);
				//yhがheightより小さいなら、centerからyh/2だけ上下に伸ばす
				int center=height/2;
				for(int h=0; h<height; ++h){
					if(h<=center-yh/2){
						//mvprintw(,2*k,"%s ",wall.c_str());
					}else if(h<center+yh/2){
						int color=0;
						switch (wd[k]){
							case 0:
								color=3;
								break;
							case 1:
								color=4;
								break;
							case 2:
								color=5;
								break;
							case 3:
								color=6;
								break;
						}
						attron(COLOR_PAIR(color));
						if(depth[k]>10){
							mvprintw(h,2*k,"%s$",wall.c_str());
						}else if(depth[k]>6){
							mvprintw(h,2*k,"%s1",wall.c_str());
						}else if(depth[k]>5){
							mvprintw(h,2*k,"%s!",wall.c_str());
						}else{
							mvprintw(h,2*k,"%s ",wall.c_str());
						}
						attroff(COLOR_PAIR(color));
					}else{
						mvprintw(h,2*k,". ");
					}
				}
			
		}else{
			for(int h=height/2; h<height; ++h){
					mvprintw(h,2*k,". ");
				}
		}
	}
}
int main(){
	complex sightdir={1,0};
	bool raymarching=false;
	string log="----Log----";
	initscr();
	cbreak();
	noecho();
	//nodelay(stdscr, TRUE); やるならnapms(16 or 32)
	start_color();
	init_pair(1,COLOR_RED,COLOR_BLACK); 
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(3,COLOR_RED,COLOR_BLACK);
	init_pair(4,COLOR_GREEN,COLOR_BLACK);
	init_pair(5,COLOR_BLUE,COLOR_BLACK);
	init_pair(6,COLOR_YELLOW,COLOR_BLACK);
	int u[2]={};
	Space<Tile> map=generate();
	randomize(map,u);
	view(map,u);
while(true){
	erase();
	if(raymarching){
		log="";
		vector<int> v;
		array<vec2,width> viewi={};
		array<double,width> depth={};
		array<int,width> walldirection={};
		//レイマーチ
		for(int k=0; k<width; ++k){
			complex imag={0,1};
			complex up={u[0]+0.5,u[1]+0.5};
			complex dir=sightdir;
			//-1~1
			double a=width;
			double dheight=height;
			complex pos=sum(mul(smul(dir,(2*k/a-1)*viewwidth),imag),up);
			complex q=sum(smul(dir,-fp),up);
			complex eps=sdiv(normalize(sub(q,pos)),-50);//sdiv(neg(mp),25*sqrt(norm(mp)));
			int iteration=1000;
			for(int i=0; i<iteration; ++i){
				pos=sum(pos,eps);
				//衝突
				int roompos[2]={static_cast<int>(floor(pos[0])),static_cast<int>(floor(pos[1]))};
				//衝突判定が怪しい
				if(touch(map,roompos,0,0).wall){
					vec2 romp={mod(roompos[0],width),mod(roompos[1],height)};
					viewi[k]=romp;
					complex post={mod(pos[0],a),mod(pos[1],dheight)};
					double d=pow(post[0]-romp[0]-0.5,2)-pow(post[1]-romp[1]-0.5,2);
					if(d>0){
						//x軸
						if(0.5<post[0]-romp[0]){
							//d
							walldirection[k]=3;
						}else{
							//a
							walldirection[k]=1;
						}
					}else{
						if(0.5<post[1]-romp[1]){
							//s
							walldirection[k]=2;
						}else{
							//w
							walldirection[k]=0;
						}
					}
					depth[k]=i/50.;
					//log+="("+to_string(depth[k])+")";
					break;
				}
				if(i==iteration-1){
					vec2 rm={-1,-1};
					viewi[k]=rm;
					depth[k]=-1;
				}
			}
		}
		//予定通りならviewiとdepthが生成される。↑に対して←
		drawWithDepth(map,viewi,depth,walldirection);
	}else{
	for(int i=0; i<height; ++i){
		for(int j=0; j<width; ++j){
			if(map[j][i].revealed){
				if(map[j][i].wall){
					if(map[j][i].weight==sight){
							draw(j,i,wall,u,2);
					}else{
						if(map[j][i].view){
							draw(j,i,wall,u,-1);
						}else{
							draw(j,i,wall,u,1);
						}
					}
				}else{
					if(map[j][i].weight==sight){
						draw(j,i,".",u,2);
					}else{
						if(map[j][i].view){
							draw(j,i,".",u,-1);
						}else{
							draw(j,i,".",u,1);
						}
					}
				}
			}
		}
	}
	}
	if(!raymarching){
		mvprintw(height/2,width,"%s ",playerch.c_str());
		if(!touch(map,u,round(sightdir[0]),round(sightdir[1])).wall){
			mvprintw(round(height/2+sightdir[1]),round(width+2*sightdir[0]),", ");
		}
	}
	mvprintw(height+1,100,"%s",log.c_str());
	refresh();
	//keyevent
	std::string c=std::string(1,getch());
	int movement=-1;
	complex moneone={1,1};
	if(raymarching){
		complex mov={0,0};
		if(c=="w"){
			mov[1]=1;
		}
		if(c=="a"){
			mov[0]=-1;
		}
		if(c=="s"){
			mov[1]=-1;
		}
		if(c=="d"){
			mov[0]=1;
		}
		if(!(mov[0]==0 && mov[1]==0)){
			double ang=arg(mul(mul(conj(sightdir),smul(moneone,sqrt(2)/2)),mov))+pi;
			movement=floor(2*ang/pi);
		}
	}else{
		if(c=="w"){
			movement=0;
		}
		if(c=="a"){
			movement=1;
		}
		if(c=="s"){
			movement=2;
		}
		if(c=="d"){
			movement=3;
		}
	}
	if(movement==0){
		if(ismovable(map,u,0,-1)){
			u[1]=mod(u[1]-1,height);
			moved(map,u);
		}
		if(!raymarching){
			sightdir[0]=0;
			sightdir[1]=-1;
		}
	}
	if(movement==1){
		if(ismovable(map,u,-1,0)){
			u[0]=mod(u[0]-1,width);
			moved(map,u);
		}
		if(!raymarching){
		sightdir[0]=-1;
		sightdir[1]=0;
		}
	}
	if(movement==2){
		if(ismovable(map,u,0,1)){
			u[1]=mod(u[1]+1,height);
			moved(map,u);
		}
		if(!raymarching){
		sightdir[0]=0;
		sightdir[1]=1;
		}
	}
	if(movement==3){
		if(ismovable(map,u,1,0)){
			u[0]=mod(u[0]+1,width);
			moved(map,u);
		}
		if(!raymarching){
		sightdir[0]=1;
		sightdir[1]=0;
		}
	}
	if(c=="z"){
		raymarching=!raymarching;
	}
	if(c=="e"){
		sightdir=mul(sightdir,poler(1,0.1));
	}
	if(c=="q"){
		sightdir=mul(sightdir,poler(1,-0.1));
	}
	if(c=="x"){
		sightdir=mul(sightdir,poler(1,-pi/2));
	}
	if(c=="c"){
		sightdir=mul(sightdir,poler(1,pi/2));
	}
}
endwin();
}

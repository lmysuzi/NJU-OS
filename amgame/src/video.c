#include <game.h>

#define SIDE 16
#define UP 30
#define LEFT 43
#define DOWN 44
#define RIGHT 45
#define BALL_SIZE 30
#define STEP_SIZE 5
#define COL_WHITE    0xeeeeee
#define COL_RED      0xff0033
#define COL_GREEN    0x00cc33
#define COL_PURPLE   0x2a0a29
static int w, h;
static int oldX,oldY;
Ball ball;

static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

/*static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}*/


static void draw_ball(int x,int y,int w,int h,uint32_t color){
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

static void ball_init(){
  oldX=ball.x=(w-BALL_SIZE)/2;
  oldY=ball.y=(h-BALL_SIZE)/2;
  draw_ball(ball.x,ball.y,BALL_SIZE,BALL_SIZE,COL_RED);
}

void move_ball(int direction){
  switch (direction)
  {
  case UP:
    if(ball.y-STEP_SIZE>=0){oldY=ball.y;ball.y-=STEP_SIZE;}
    if(oldY==ball.y)printf("no\n");
    break;
  case DOWN:
    if(ball.y+STEP_SIZE<h)oldY=ball.y,ball.y+=STEP_SIZE;
    break;
  case LEFT:
    if(ball.x-STEP_SIZE>=0)oldX=ball.x,ball.x-=STEP_SIZE;
    break;
  case RIGHT:
    if(ball.x+STEP_SIZE<w)oldX=ball.x,ball.x+=STEP_SIZE;
    break;
  default:
    break;
  }
}

void update(){
  if(oldX==ball.x&&oldY==ball.y)return;
  draw_ball(oldX,oldY,BALL_SIZE,BALL_SIZE,COL_GREEN);
  draw_ball(ball.x,ball.y,BALL_SIZE,BALL_SIZE,COL_RED);
}

void splash() {
  init();
  ball_init();
  /*for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }*/
}

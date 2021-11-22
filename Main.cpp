#include "Utilities.h"
#include "Solver.h"

extern struct Sim_Struct sim_struct;
extern struct Info_Struct info_struct;

std::chrono::high_resolution_clock::time_point begin,end;
std::chrono::microseconds duration;

void update(int value) {

  begin = std::chrono::high_resolution_clock::now();

  run_sim_timestep();

  if (info_struct.run_graphics) {
    glutPostRedisplay();
    glutTimerFunc(info_struct.framerate, update, 0);
  }

  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
  std::cout << "Runtime: " << duration.count()/1000. << " milliseconds" << std::endl;;
}

int main(int argc, char const *argv[]) {

  read_config();

  read_grid_and_init_struct();

  printf("Framerate: %d\n", info_struct.framerate);
  printf("Grid size: %d x %d\n", sim_struct.grid_size_x, sim_struct.grid_size_y);
  printf("Delta t: %lf\n", sim_struct.dt);
  printf("Delta x: %lf\n", sim_struct.dx);
  printf("Delta y: %lf\n", sim_struct.dy);
  printf("Viscosity: %lf\n", sim_struct.mu);
  printf("Speed of sound: %lf\n", sim_struct.c);

  printf("Maximum allowed timestep by Courant stability: %lf\n", sim_struct.dx/sim_struct.c);

  if (!info_struct.run_graphics) {
    while (true) {
      update(0);
    }
  } else {
    // GL initialization
    int dummy_thicc = 0;
    glutInit(&dummy_thicc, NULL);
    glutInitDisplayMode(GLUT_SINGLE);

    //initial window size and position
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(1100, 200);

    //Window title and declaration of draw function
    glutCreateWindow("");
    glutDisplayFunc(render);
    glutTimerFunc(info_struct.framerate, update, 0);
    glutKeyboardFunc(leave_glut);
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    //returns you back to main() after simlation is over
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutMainLoop();
  }



  return 0;
}

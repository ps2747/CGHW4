g++ -o HW4 -static -O2 -std=c++0x -DGLEW_STATIC main.cpp tiny_obj_loader.cc libglew.a -I.  -lglfw3 -lopengl32 -lgdi32 
strip HW4.exe
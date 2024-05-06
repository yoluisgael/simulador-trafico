#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <set>
#include <functional>
#include <fstream>
#include "gnuplot.h"
using namespace std;

int seed = time(NULL);
const int gridx = 39;
const int gridy = 20;
const int cellsize = 48;
const vector<bool> reglas = {0,0,0,1,1,1,0,1};
enum Direction {UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3};
enum Type {BLUE = 0, NORMAL = 1, BIG = 2, MOTORCYCLE = 3};
vector<int> tru = {1};
vector<int> fols = {0, 0};
vector<int> random_spawn{0};
bool priority = false;
bool trabar_switch = true;
int parking_prob = 30;
int turn_prob = 35;
int spawn_prob = 0;
int trabar_prob = 1;
int trabados = 0;


class Road{
public:
    vector<int> r;
    int x, y;
    vector<int>::iterator prev, next;
    map<int, vector<int>::iterator> intersections;
    set<int> turns, empty_turns;
    Direction direction;
    Road(int x, int y, int size, Direction dir, vector<int>::iterator prev = fols.begin(), vector<int>::iterator next = tru.begin())
    : x(x), y(y), direction(dir), prev(prev), next(next){
        r.resize(size, 0);
    }
};
vector<Road> roads;
vector<Road> temp;
vector<int> entrada;

class Parking{
public:
    vector<pair<int, vector<int>::iterator>> p;
    Direction direction, drawdirection;
    Type type;
    int x,y;

    Parking(int x, int y, int size, Direction dir, Direction draw, Type type, Road &r, int start)
    : x(x), y(y), direction(dir), drawdirection(draw), type(type){
        p.resize(size);
        if(this->type == MOTORCYCLE){
            for(int i = 0; i < size; i++)
                p[i] = {0, r.r.begin() + start};
        }
        else{
            for(int i = 0; i < size; i++)
                p[i] = {0, r.r.begin() + start + i};
        }
    }
};
vector<Parking> parking_slots;

//Unir calles, fin a principio
void join_roads(Road &road1, Road &road2){
    road1.next = road2.r.begin();
    road2.prev = road1.r.end()-1;
}

//Crear intersecciones entre calles
void intersect_roads(Road &road1, int road1crossing, Road &road2, int road2crossing){
    if(road2crossing < 1) {
        road1.intersections[road1crossing - 1] = road2.r.begin();
        road1.empty_turns.insert(road1crossing - 1);
    }
    else if(road1crossing < 1) {
        road2.intersections[road2crossing - 1] = road1.r.begin();
        road2.empty_turns.insert(road2crossing - 1);
    }
    else {
        road1.intersections[road1crossing - 1] = road2.r.begin() + road2crossing;
        road2.intersections[road2crossing - 1] = road1.r.begin() + road1crossing;
    }
}

//Dar vuelta a los carros disponibles
void turn_cars(Road &road){
    int prob = turn_prob;
    for(auto turn: road.turns){
        if(road.r[turn] and
        // Si es el fin de la calle está forzado a dar vuelta
          (rand() % 100 < prob or (road.r.begin() + turn + 1) == road.r.end())){
            int temp = road.r[turn];
            road.r[turn] = false;
            *road.intersections[turn - 1] = temp;
        }
    }
}

void forzar_entrada(){
    if(roads[7].r[26] % 2 == 1 and rand() % 100 < turn_prob){
        roads[0].r[0] = roads[7].r[26];
        roads[7].r[26] = 0;
    }
    if(roads[8].r[9] % 2 == 1 and rand() % 100 < turn_prob){
        roads[0].r[1] = roads[8].r[9];
        roads[8].r[9] = 0;
    }
}

//Inicializar las calles
void create_roads(){
    roads.push_back(Road(11, 1, 13, DOWN));
    roads.push_back(Road(19, 5, 9, DOWN));
    roads.push_back(Road(28, 13, 13, UP));

    roads.push_back(Road(36,5, 34, LEFT));
    roads.push_back(Road(3,6, 7, DOWN));
    roads.push_back(Road(3,13,34,RIGHT));
    roads.push_back(Road(36,12,7,UP));

    roads.push_back(Road(37,1,36,LEFT));
    roads.push_back(Road(2,2,36,RIGHT));

    roads[7].prev = random_spawn.begin();
    roads[8].prev = random_spawn.begin();               // Inicio
    roads[7].next = fols.begin();                       // Fin
    roads[8].next = fols.begin();

    join_roads(roads[3], roads[4]);
    join_roads(roads[4], roads[5]);
    join_roads(roads[5], roads[6]);
    join_roads(roads[6], roads[3]);

    intersect_roads(roads[0], 4, roads[3], 25);
    intersect_roads(roads[0], 12, roads[5], 8);
    intersect_roads(roads[1], 0, roads[3], 17);
    intersect_roads(roads[1], 8, roads[5], 16);
    intersect_roads(roads[2], 8, roads[3], 8);
    intersect_roads(roads[2], 0, roads[5], 25);

    intersect_roads(roads[0], 0, roads[7], 26);
    intersect_roads(roads[0], 1, roads[8], 9);
    intersect_roads(roads[2], 11, roads[8], 26);
    intersect_roads(roads[2], 12, roads[7], 9);

    roads[0].turns = {4, 12};
    roads[1].turns = {8};
    roads[2].turns = {8, 11, 12};
    roads[3].turns = {8, 17, 25};
    roads[4].turns = {};
    roads[5].turns = {25};
    roads[6].turns = {};
    roads[7].turns = {26};
    roads[8].turns = {9};

    temp = roads;
    entrada.resize(roads[5].r.size(), 0);
}

// Inicializar los lugares de estacionamiento
void create_parking(){
    parking_slots.push_back(Parking(5, 12, 4, UP, RIGHT, BLUE, roads[5], 2));
    parking_slots.push_back(Parking(5, 14, 4, DOWN, RIGHT, BLUE, roads[5], 2));
    parking_slots.push_back(Parking(13, 12, 4, UP, RIGHT, BIG, roads[5], 10));
    parking_slots.push_back(Parking(14, 14, 4, DOWN, RIGHT, BIG, roads[5], 11));
    parking_slots.push_back(Parking(19,14,17,DOWN,RIGHT,NORMAL,roads[5],16));
    parking_slots.push_back(Parking(10,4,7,UP,LEFT,NORMAL,roads[3], 26));
    parking_slots.push_back(Parking(2,6,5,LEFT,DOWN,NORMAL,roads[4],0));
    parking_slots.push_back(Parking(2,5,1,LEFT,DOWN,NORMAL,roads[3],33));
    parking_slots.push_back(Parking(9,6,5,DOWN,LEFT,NORMAL,roads[3],27));
    parking_slots.push_back(Parking(4,7,5,RIGHT,DOWN,NORMAL,roads[4],2));
    parking_slots.push_back(Parking(10,7,5,LEFT,DOWN,NORMAL,roads[0],6));
    parking_slots.push_back(Parking(26,4,14,UP,LEFT,NORMAL,roads[3],10));
    parking_slots.push_back(Parking(17,6,4,DOWN,LEFT,BIG,roads[3],19));
    parking_slots.push_back(Parking(12,7,5,RIGHT,DOWN,NORMAL,roads[0],6));
    parking_slots.push_back(Parking(18,7,5,LEFT,DOWN,NORMAL,roads[1],2));
    parking_slots.push_back(Parking(26,6,6,DOWN,LEFT,NORMAL,roads[3],10));
    parking_slots.push_back(Parking(20,7,5,RIGHT,DOWN,NORMAL,roads[1],2));
    parking_slots.push_back(Parking(21,12,6,UP,RIGHT,NORMAL,roads[5],18));
    parking_slots.push_back(Parking(27,11,5,LEFT,UP,NORMAL,roads[2],2));
    parking_slots.push_back(Parking(29,11,5,RIGHT,UP,NORMAL,roads[2],2));
    parking_slots.push_back(Parking(30,12,5,UP,RIGHT,NORMAL,roads[5],27));
    parking_slots.push_back(Parking(35,11,5,LEFT,UP,NORMAL,roads[6],1));
    parking_slots.push_back(Parking(34,6,5,DOWN,LEFT,NORMAL,roads[3],2));
    parking_slots.push_back(Parking(36,4,7,UP,LEFT,NORMAL,roads[3],0));
    parking_slots.push_back(Parking(37,12,7,RIGHT,UP,NORMAL,roads[6],0));
    parking_slots.push_back(Parking(12,14,5,LEFT,DOWN,MOTORCYCLE,roads[5],9));
    parking_slots.push_back(Parking(13,14,5,RIGHT,DOWN,MOTORCYCLE,roads[5],10));
    parking_slots.push_back(Parking(2,11,5,UP,LEFT,MOTORCYCLE,roads[4],5));
    parking_slots.push_back(Parking(2,12,5,DOWN,LEFT,MOTORCYCLE,roads[4],6));
    parking_slots.push_back(Parking(27,12,3,RIGHT,UP, MOTORCYCLE,roads[5],23));
    parking_slots.push_back(Parking(35,12,3,RIGHT,UP, MOTORCYCLE,roads[5],31));
}

// Calcular regla según la vecindad
int regla_actual(int cell, Road &r){
    int resultado = 0;

    //Checar la continuación de otra calle anterior
    if(cell-1 < 0)
        resultado += 4 *  (*r.prev ? 1 : 0);
    else
        resultado += 4 * (r.r[cell-1] ? 1 : 0);

    resultado += 2 * (r.r[cell] ? 1 : 0);

    if(cell+1 > r.r.size()-1)
        resultado += (*r.next ? 1 : 0);
    else
        resultado += (r.r[cell+1] ? 1 : 0);
    return resultado;
}

// Calcular la regla si es que hay un carro detenido
int regla_trabada(int cell){
    int x = 0;

    //Checar la continuación de otra calle anterior
    if(cell-1 < 0){
        x += 4 *  (*roads[5].prev ? 1 : 0);
    }
    //Si el anterior está detenido, no avanzará a la celda actual
    else{
        if(entrada[cell-1] <= 0)
            x += 4 * (roads[5].r[cell-1] ? 1 : 0);
    }

    x += 2 * (roads[5].r[cell] ? 1 : 0);

    // Si está detenido, no avanzará
    if(entrada[cell] > 0){
        x++;
        entrada[cell]--;
    }
    // Si no está detenido, avanzará
    else{
        if(cell+1 > roads[5].r.size()-1)
            x += (*roads[5].next ? 1 : 0);
        else
            x += (roads[5].r[cell+1] ? 1 : 0);
    }

    // Mismo estado
    if(x == 3 or x == 7){
        return roads[5].r[cell];
    }
        // Estado de la celda anterior
    else if(x == 4 or x == 5){
        if(cell - 1 < 0)
            return *roads[5].prev;
        else
            return roads[5].r[cell-1];
    }
    // Estado 0
    return 0;
}

//Checar prioridad de paso
bool prioridad(Road &r){
    if(priority % 2 == 0){
        if(r.direction == DOWN or r.direction == UP)
            return true;
    }
    else{
        if(r.direction == RIGHT or r.direction == LEFT)
            return true;
    }
    return false;
}

//Checar si se puede cruzar
bool cruce(Road &road, int cell){
    if(road.intersections.find(cell) != road.intersections.end() and road.r[cell] != 0){
        //Si hay alguien en frente, estorbando, no hacer nada
        if(*road.intersections[cell] != 0){
            return true;
        }
        //Si alguien avanzará al mismo tiempo, se dará prioridad uno a uno
        else if(road.empty_turns.find(cell) == road.empty_turns.end()){
            if(*(road.intersections[cell] - 1) and prioridad(road)) {
                priority = !priority;
                return true;
            }
        }
    }
    //Sino, aplicar regla
    return false;
}

//Decide el tiempo que permanecerá un carro estacionado
int tiempo_a_estacionar(int horas){
    int n = 19 - horas;
    if(n <= 0)
        return 1;
    if(n < 5)
        return 1 + rand() % n;
    int a = rand() % n;
    if(a <= n/5)
        return 1 + rand() % (n/5);
    if(a <= (4*n/5))
        return (n/5) + (rand() % (3*n/5));
    return (4*n/5) + rand() % (n/5);
}

//Manejar el estacionamiento de los carros
void estacionar(int horas){
    int prob = parking_prob;
    for(Parking &p: parking_slots) {
        for (auto &lugar: p.p) {
            //MOTOS
            if(p.type == MOTORCYCLE){
                // Permite estacionar motos entrando
                if(lugar.first <= 0 and (*lugar.second == 3)){
                    if (rand() % 100 < prob*1.5) {
                        lugar.first = tiempo_a_estacionar(horas) * 60 * 4;
                        *lugar.second = 0;
                    }
                }
                // Da salida si no hay nadie, con estado de salida
                else if (lugar.first == 1 and !*lugar.second){
                    lugar.first = 0;
                    *lugar.second = 4;
                }
                // Si aún no sale, reduce el tiempo
                else if (lugar.first > 1)
                    lugar.first--;
            }
            //CARROS
            // Solo permite estacionar carros entrando
            else if (lugar.first <= 0 and (*lugar.second == 1)) {
                if (rand() % 100 < prob) {
                    lugar.first = tiempo_a_estacionar(horas) * 60 * 4;
                    *lugar.second = 0;
                }
            }
            // Da salida si no hay nadie en la calle, con estado de salida
            else if (lugar.first == 1 and !*lugar.second) {
                lugar.first = 0;
                *lugar.second = 2;
            }
            // Si aún no busca salir, se reduce el tiempo
            else if (lugar.first > 1) {
                lugar.first--;
            }
        }
    }
}

// Aplicar la regla para los distintos estados
// Permite usar estados de carros, motos, entrada y salida
int new_rule(int cell, Road &r){
    int x = regla_actual(cell, r);
    // Mismo estado
    if(x == 3 or x == 7){
        return r.r[cell];
    }
    // Estado de la celda anterior
    else if(x == 4 or x == 5){
        if(cell - 1 < 0)
            return *r.prev;
        else
            return r.r[cell-1];
    }
    // Estado 0
    return 0;
}

// Evitar vueltas a los carros que salen
void forzar_salida(){
    if(roads[5].r[25] == 2 or roads[5].r[25] == 4){
        roads[2].r[0] = roads[5].r[25];
        roads[5].r[25] = 0;
    }
    if(roads[3].r[8] == 2 or roads[3].r[8] == 4){
        roads[2].r[8] = roads[3].r[8];
        roads[3].r[8] = 0;
    }
    if(roads[3].r[8] == 3 and rand() % 100 < 20){
        roads[2].r[8] = roads[3].r[8];
        roads[3].r[8] = 0;
    }
    if(roads[0].r[0] != 0 and roads[0].r[0] % 2 == 0){
        roads[7].r[26] = roads[0].r[0];
        roads[0].r[0] = 0;
    }
}

// Busca evitar la salida
void forzar_estacionamiento(){
    if(roads[2].r[8] == 1 and rand()% 100 < 98){
        roads[3].r[8] = roads[2].r[8];
        roads[2].r[8] = 0;
    }
    if(roads[2].r[8] == 3 and rand()% 100 < 30){
        roads[3].r[8] = roads[2].r[8];
        roads[2].r[8] = 0;
    }
}

// Cambiar la hora cada paso
void avanzar_tiempo(int &horas, int &minutos, int &segundos){
    if(segundos < 5 and !(horas >= 22 or horas < 6 or horas == 6 and minutos < 30)){
        segundos++;
    }
    else {
        segundos = 0;
        if(minutos < 59)
            if(horas >= 22 or (horas < 6 or horas == 6 and minutos < 30)) minutos += 5;
            else minutos++;
        else{
            minutos = 0;
            if(horas < 23)
                horas++;
            else
                horas = 0;
        }
    }
}

// Trabar en la calle de la entrada
void trabar(){
    for(int i=2; i<entrada.size() - 2; i++){
        int x = rand() % 19999;
        if(roads[5].r[i] > 0  and entrada[i] <= 0 and x < trabar_prob){
            entrada[i] = (7 + rand() % 8) * 5 + rand() % 5;
            cout << ++trabados << endl;
        }
    }
}

//Aplicar la regla 186 para avanzar a los carros
void step(int horas){
    for(int i=0; i<roads.size(); i++) {
        if(trabar_switch and horas < 20)
            trabar();
        for (int cell = 0; cell < roads[i].r.size(); cell++){
            if(i == 7){
                roads[0].r[0] = temp[0].r[0];
            }
            if(i == 8){
                roads[0].r[1] = temp[0].r[1];
                priority = !priority;
            }
            if(trabar_switch and i == 5){
                if(!cruce(roads[i], cell)){
                    temp[i].r[cell] = regla_trabada(cell);
                }
                else{
                    if(roads[i].r[cell])
                        entrada[cell]--;
                    temp[i].r[cell] = roads[i].r[cell];
                    roads[i].r[cell] = 0;
                }
            }
            //Si hay un cruce, checar si se puede avanzar, sino, no avanzar
            else if(!cruce(roads[i], cell))
                temp[i].r[cell] = new_rule(cell, roads[i]);
            else {
                temp[i].r[cell] = roads[i].r[cell];
                roads[i].r[cell] = 0;
            }
        }
    }

    //Aplicar cambios
    roads = temp;

    //Dar vueltas
    for(auto &r: roads)
        turn_cars(r);
    forzar_salida();
    forzar_estacionamiento();
    forzar_entrada();
}

//Inicializar calles aleatoriamente
void random_start(int horas){
    int prob = 50;
    for(auto &r: roads)
        for (int j = 0; j < r.r.size(); j++)
            r.r[j] = (rand() % 100 < prob) * (1+ rand()% 4);
    for(auto &r: roads) {
        for (auto &c: r.intersections) {
            if(r.r[c.first + 1] and *c.second){
                if(rand() % 100 < 50){
                    r.r[c.first] = false;
                }
                else {
                    *c.second = false;
                }
            }
        }
    }
    for(auto &p: parking_slots)
        for(auto &s: p.p)
            estacionar(horas);
}

//Dibujar calle
void draw_road(Road &r, sf::RenderWindow &fondo){
    sf::Color gray(128,128,128);
    sf::RectangleShape cell(sf::Vector2f(cellsize, cellsize));
    cell.setOutlineThickness(0);
    cell.setFillColor(gray);
    sf::RectangleShape horizontal(sf::Vector2f(cellsize * 2 / 7, cellsize/7));
    horizontal.setFillColor(sf::Color::Yellow);
    sf::RectangleShape vertical(sf::Vector2f(cellsize / 7, cellsize * 2 /7));
    vertical.setFillColor(sf::Color::Yellow);
    if(r.direction == UP){
        for(int i = r.y, j = 0; j < r.r.size(); i--, j++){
            cell.setPosition(r.x * cellsize, 40 + i * cellsize);
            fondo.draw(cell);
            vertical.setPosition(r.x * cellsize + (cellsize*3/7), 40 + i * cellsize + (cellsize/7));
            fondo.draw(vertical);
            vertical.setPosition(r.x * cellsize + (cellsize*3/7), 40 + i * cellsize + (cellsize*4/7));
            fondo.draw(vertical);
        }
    } else if(r.direction == RIGHT){
        for(int i = r.x, j = 0; j < r.r.size(); i++, j++){
            cell.setPosition(i * cellsize, 40 + r.y * cellsize);
            fondo.draw(cell);
            horizontal.setPosition(i * cellsize + (cellsize/7), 40 + cellsize * r.y + (cellsize*3/7));
            fondo.draw(horizontal);
            horizontal.setPosition(i * cellsize + (cellsize*4/7), 40 + cellsize * r.y + (cellsize*3/7));
            fondo.draw(horizontal);
        }
    } else if(r.direction == DOWN){
        for(int i = r.y, j = 0; j < r.r.size(); i++, j++){
            cell.setPosition(r.x * cellsize, 40 + i * cellsize);
            fondo.draw(cell);
            vertical.setPosition(r.x * cellsize + (cellsize*3/7), 40 + i * cellsize + (cellsize/7));
            fondo.draw(vertical);
            vertical.setPosition(r.x * cellsize + (cellsize*3/7), 40 + i * cellsize + (cellsize*4/7));
            fondo.draw(vertical);
        }
    } else if(r.direction == LEFT){
        for(int i = r.x, j = 0; j < r.r.size(); i--, j++){
            cell.setPosition(i * cellsize, 40 + r.y * cellsize);
            fondo.draw(cell);
            horizontal.setPosition(i * cellsize + (cellsize/7), 40 + cellsize * r.y + (cellsize*3/7));
            fondo.draw(horizontal);
            horizontal.setPosition(i * cellsize + (cellsize*4/7), 40 + cellsize * r.y + (cellsize*3/7));
            fondo.draw(horizontal);

        }
    }
}

//Dibujar estacionamiento
void draw_parking_slots(Parking &p, sf::RenderWindow &window, sf::Sprite &sprite){
    int size = p.p.size();
    sf::Color color(128, 128, 128);
    sf::Color gray(128,128,128);
    sf::RectangleShape cell(sf::Vector2f(cellsize, cellsize));
    cell.setOutlineThickness(0);
    cell.setFillColor(gray);
    if(p.type == MOTORCYCLE){
        if(p.direction == UP or p.direction == DOWN){
            sf::RectangleShape cell(sf::Vector2f(cellsize/3 - 4, cellsize - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            if(p.drawdirection == LEFT){
                for (int i = 0; i > -size; i--) {
                    cell.setPosition(2+cellsize * p.x + (cellsize * i/3) + (cellsize*2/3), 42 + p.y * cellsize);
                    window.draw(cell);
                }
            } else {
                for (int i = 0; i < size; i++) {
                    cell.setPosition(2+cellsize * p.x + (cellsize * i/3), 42 + p.y * cellsize);
                    window.draw(cell);
                }
            }
        } else {
            sf::RectangleShape cell(sf::Vector2f(cellsize - 4, cellsize/3 - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            if(p.drawdirection == UP){
                for (int i = 0; i > -size; i--) {
                    cell.setPosition(2+cellsize * p.x, 42 + p.y * cellsize  + (cellsize * i/3) + (cellsize*2/3));
                    window.draw(cell);
                }
            } else {
                for (int i = 0; i < size; i++) {
                    cell.setPosition(2+cellsize * p.x, 42 + p.y * cellsize + (cellsize * i/3));
                    window.draw(cell);
                }
            }
        }
    }
    if(p.type == BLUE){
        sf::FloatRect bounds = sprite.getGlobalBounds();
        sf::Sprite up, down;
        up = sprite;
        down = sprite;
        down.setOrigin(bounds.width, bounds.height);
        down.rotate(180.0f);
        if(p.direction == UP){
            for(int i = 0; i < p.p.size(); i++){
                up.setPosition(cellsize * p.x + (cellsize * i *5/4), 40 + p.y * cellsize);
                window.draw(up);
            }
        }
        else if(p.direction == DOWN) {
            for (int i = 0; i < p.p.size(); i++) {
                down.setPosition(cellsize * p.x + (cellsize * i *5/4), 40 + p.y * cellsize);
                window.draw(down);
            }
        }
    }
    if(p.type == BIG){
        if(p.drawdirection == RIGHT){
            sf::RectangleShape cell(sf::Vector2f(cellsize * 5/4 - 4, cellsize - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            for(int i = 0; i < size; i++){
                cell.setPosition(2+cellsize * p.x + (cellsize * i *5/4), 42 + p.y * cellsize);
                window.draw(cell);
            }
        }
        else if(p.drawdirection == LEFT) {
            sf::RectangleShape cell(sf::Vector2f(cellsize * 5/4 - 4, cellsize - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            for (int i = 0; i > -size; i--) {
                cell.setPosition(2+cellsize * p.x + (cellsize * i *5/4) - (cellsize / 4), 42 + p.y * cellsize);
                window.draw(cell);
            }
        }
        else if(p.direction == UP){
            sf::RectangleShape cell(sf::Vector2f(cellsize - 4, cellsize * 5/4 - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            for (int i = 0; i> -size; i--) {
                cell.setPosition(2+cellsize * p.x, 42 + cellsize * p.y + (cellsize * i *5/4) - cellsize/4);
                window.draw(cell);
            }
        }
        else if(p.drawdirection == DOWN){
            sf::RectangleShape cell(sf::Vector2f(cellsize - 4, cellsize * 5/4 - 4));
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Yellow);
            cell.setFillColor(gray);
            for(int i = 0; i < size; i++){
                cell.setPosition(2+cellsize * p.x, 42 + cellsize * p.y + (cellsize * i *5/4));
                window.draw(cell);
            }
        }
    }
    if(p.type == NORMAL){
        sf::RectangleShape cell(sf::Vector2f(cellsize - 4, cellsize - 4));
        cell.setOutlineThickness(2);
        cell.setOutlineColor(sf::Color::Yellow);
        cell.setFillColor(gray);
        if(p.drawdirection == RIGHT){
            for(int i = 0; i < size; i++){
                cell.setPosition(2+cellsize * p.x + (cellsize * i), 42 + p.y * cellsize);
                window.draw(cell);
            }
        }
        else if(p.drawdirection == LEFT) {
            for (int i = 0; i > -size; i--) {
                cell.setPosition(2+cellsize * p.x + (cellsize * i), 42 + p.y * cellsize);
                window.draw(cell);
            }
        }
        else if(p.drawdirection == UP){
            for (int i = 0; i > -size; i--) {
                cell.setPosition(2+cellsize * p.x, 42 + cellsize * p.y + (cellsize * i));
                window.draw(cell);
            }
        }
        else if(p.drawdirection == DOWN){
            for(int i = 0; i < size; i++){
                cell.setPosition(2+cellsize * p.x, 42 + cellsize * p.y + (cellsize * i));
                window.draw(cell);
            }
        }
    }
}

//Dibujar carros estacionados
void draw_parked_cars(Parking &p, sf::RenderWindow &window, sf::Sprite &sprite1, sf::Sprite &sprite2){
    int size = p.p.size();
    sf::FloatRect bounds1 = sprite1.getGlobalBounds();
    sf::FloatRect bounds2 = sprite1.getGlobalBounds();
    sf::Sprite car = sprite1;
    sf::Sprite moto = sprite2;
    if(p.type == MOTORCYCLE){
        if(p.direction == LEFT){
            moto.setOrigin(bounds2.width, bounds2.height);
            moto.rotate(180.0f);
        } else if (p.direction == DOWN){
            moto.setOrigin(bounds2.width - cellsize, bounds2.height);
            moto.rotate(90.0f);
        } else if(p.direction == UP){
            moto.setOrigin(bounds2.width, bounds2.height - cellsize);
            moto.rotate(270.0f);
        }
        if(p.drawdirection == LEFT){
            for(int i=0, j=0; i > -size; i--, j++){
                if(p.p[j].first > 0) {
                    moto.setPosition(2 + cellsize * p.x + (cellsize * i / 3) + (cellsize * 2 / 8), 42 + cellsize * p.y);
                    window.draw(moto);
                }
            }
        } else if(p.drawdirection == RIGHT){
            for(int i=0, j=0; i < size; i++, j++){
                if(p.p[j].first > 0) {
                    moto.setPosition(2 + cellsize * p.x + cellsize * i / 3, 42 + cellsize * p.y);
                    window.draw(moto);
                }
            }
        } else if(p.drawdirection == UP){
            for(int i=0, j=0; i > -size; i--, j++){
                if(p.p[j].first > 0) {
                    moto.setPosition(2 + cellsize * p.x, 42 + cellsize * p.y + (cellsize * i / 3) + (cellsize/ 3));
                    window.draw(moto);
                }
            }
        } else {
            for(int i=0, j=0; i < size; i++, j++){
                if(p.p[j].first > 0) {
                    moto.setPosition(2 + cellsize * p.x, 42 + cellsize * p.y + (cellsize * i / 3) - (cellsize * 3 / 8));
                    window.draw(moto);
                }
            }
        }

    }
    else {
        if(p.direction == LEFT){
            car.setOrigin(bounds1.width, bounds1.height);
            car.rotate(180.0f);
        } else if (p.direction == DOWN){
            car.setOrigin(bounds1.width - cellsize, bounds1.height);
            car.rotate(90.0f);
        } else if(p.direction == UP){
            car.setOrigin(bounds1.width, bounds1.height - cellsize);
            car.rotate(270.0f);
        }
        if(p.type == BLUE or p.type == BIG){
            if(p.drawdirection == RIGHT){
                for(int i = p.x, j = 0; j < p.p.size(); i++, j++){
                    if(p.p[j].first > 0){
                        car.setPosition(i * cellsize +(cellsize*j/4)+5, 40 + p.y * cellsize);
                        window.draw(car);
                    }
                }
            }
            else if(p.drawdirection == LEFT) {
                for (int i = p.x, j = 0; j < p.p.size(); i--, j++) {
                    if (p.p[j].first > 0) {
                        car.setPosition(i * cellsize - (cellsize*j/4)-5, 40 + p.y * cellsize);
                        window.draw(car);
                    }
                }
            }
        }

        else if(p.drawdirection == UP){
            for(int i = p.y, j = 0; j < p.p.size(); i--, j++){
                if(p.p[j].first > 0){
                    car.setPosition(p.x * cellsize, 40 + i * cellsize);
                    window.draw(car);
                }
            }
        } else if(p.drawdirection == RIGHT){
            for(int i = p.x, j = 0; j < p.p.size(); i++, j++){
                if(p.p[j].first > 0){
                    car.setPosition(i * cellsize, 40 + p.y * cellsize);
                    window.draw(car);
                }
            }
        } else if(p.drawdirection == DOWN){
            for(int i = p.y, j = 0; j < p.p.size(); i++, j++){
                if(p.p[j].first > 0){
                    car.setPosition(p.x * cellsize, 40 + i * cellsize);
                    window.draw(car);
                }
            }
        } else if(p.drawdirection == LEFT) {
            for (int i = p.x, j = 0; j < p.p.size(); i--, j++) {
                if (p.p[j].first > 0) {
                    car.setPosition(i * cellsize, 40 + p.y * cellsize);
                    window.draw(car);
                }
            }
        }
    }
}

//Dibujar carros
void draw_cars(Road &r, sf::RenderWindow &window, sf::Sprite &sprite1, sf::Sprite &sprite2){
    sf::Sprite carro = sprite1;
    sf::Sprite moto = sprite2;
    sf::FloatRect bounds1 = carro.getGlobalBounds();
    sf::FloatRect bounds2 = moto.getGlobalBounds();
    if(r.direction == UP){
        carro.setOrigin(bounds1.width, bounds1.height - cellsize);
        carro.rotate(270.0f);
        moto.setOrigin(bounds2.width, bounds2.height - cellsize);
        moto.rotate(270.0f);
        for(int i = r.y, j = 0; j < r.r.size(); i--, j++){
            if(r.r[j] >= 1 and r.r[j] <= 2){
                carro.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(carro);
            }
            else if(r.r[j] >= 3 and r.r[j] <= 4){
                moto.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(moto);
            }
        }
    } else if(r.direction == RIGHT){
        for(int i = r.x, j = 0; j < r.r.size(); i++, j++){
            if(r.r[j] >= 1 and r.r[j] <= 2){
                carro.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(carro);
            }
            else if(r.r[j] >= 3 and r.r[j] <= 4){
                moto.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(moto);
            }
        }
    } else if(r.direction == DOWN){
        carro.setOrigin(bounds1.width - cellsize, bounds1.height);
        carro.rotate(90.0f);
        moto.setOrigin(bounds2.width - cellsize, bounds2.height);
        moto.rotate(90.0f);
        for(int i = r.y, j = 0; j < r.r.size(); i++, j++){
            if(r.r[j] >= 1 and r.r[j] <= 2){
                carro.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(carro);
            }
            else if(r.r[j] >= 3 and r.r[j] <= 4){
                moto.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(moto);
            }
        }
    } else if(r.direction == LEFT) {
        carro.setOrigin(bounds1.width, bounds1.height);
        carro.rotate(180.0f);
        moto.setOrigin(bounds2.width, bounds2.height);
        moto.rotate(180.0f);
        for (int i = r.x, j = 0; j < r.r.size(); i--, j++) {
            if(r.r[j] >= 1 and r.r[j] <= 2){
                carro.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(carro);
            }
            else if(r.r[j] >= 3 and r.r[j] <= 4){
                moto.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(moto);
            }
        }
    }
}

//Cambiar estado de celda
bool toggleCellState(Road &r, int x, int y, bool off) {
    int cellX = x / cellsize;
    int cellY = y / cellsize;
    if (r.direction == UP && cellY >= (r.y - r.r.size() + 1)  && cellY <= r.y && cellX == r.x){
        r.r[r.y - cellY] = off ? 0 : !r.r[r.y - cellY];
        return true;
    }

    if (r.direction == RIGHT && cellX >= r.x && cellX <= (r.x + r.r.size()) && cellY == r.y) {
        r.r[cellX - r.x] = off ? 0 : !r.r[cellX - r.x];
        return true;
    }
    if (r.direction == DOWN && cellY >= r.y && cellY <= (r.y + r.r.size()) && cellX == r.x) {
        r.r[cellY - r.y] = off ? 0 : !r.r[cellY - r.y];
        return true;
    }
    if (r.direction == LEFT && cellX >= (r.x - r.r.size() + 1) && cellX <= r.x && cellY == r.y) {
        r.r[r.x - cellX] = off ? 0 : !r.r[r.x - cellX];
        return true;
    }
    return false;
}

// Borrar calles y estacionamiento
void erase(){
    for (Road &r : roads)
        fill(r.r.begin(), r.r.end(), 0);
    for (Parking &p: parking_slots)
        for(auto &s: p.p)
            s.first = 0;
}

// Probabilidad de entrada por hora
int probabilidad_por_hora(int horas, int minutos){
    if(horas == 6 and minutos >= 30)
        return 57;
    if(horas >= 7 and horas < 9)
        return 75;
    if(horas >= 9 and horas < 13)
        return 9;
    if(horas >= 13 and horas < 15)
        return 18;
    if(horas >= 15 and horas < 18)
        return 4;
    if(horas >= 18 and horas < 20)
        return 2;
    return 0;
}

// Aparecer carros
void spawn(int prob){
    if(rand() % 100 < prob){
        if(rand() % 100 < 80)
            random_spawn[0] = 1;
        else
            random_spawn[0] = 3;
    } else {
        random_spawn[0] = 0;
    }
}

// Cargar y ajustar tamaño de imagen
sf::Texture loadImage(string imageFilePath, int newWidth, int newHeight){
    sf::Texture texture;
    sf::Image originalImage;
    if (!originalImage.loadFromFile(imageFilePath)) {
        cerr << "No existe la imagen en: " << imageFilePath << endl;
        exit(-1);
    }

    unsigned int originalWidth = originalImage.getSize().x;
    unsigned int originalHeight = originalImage.getSize().y;


    sf::Image resizedImage;
    resizedImage.create(newWidth, newHeight);

    for (unsigned int x = 0; x < newWidth; ++x) {
        for (unsigned int y = 0; y < newHeight; ++y) {
            unsigned int origX = (x * originalWidth) / newWidth;
            unsigned int origY = (y * originalHeight) / newHeight;

            sf::Color pixelColor = originalImage.getPixel(origX, origY);
            resizedImage.setPixel(x, y, pixelColor);
        }
    }

    texture.loadFromImage(resizedImage);
    return texture;
}

// Graficar los datos
void plot(int max_ones, string prob){
    gnuplot p;
    string command, name;
    vector<string> plots = {"Calles", "Estacionamiento", "Total"};
    //Formato
    p("set term png");
    //Alcance
    command = "set xrange [0:";
    command += to_string(24*50);
    command += "]";
    p(command);
    //Nombre
    name += "P";
    name += prob;
    for(string s: plots){
        command = "set output \"C:/traficoEscom/plots/";
        command += name;
        command += s;
        command += ".png\"";
        p(command);
        command = "set title \"";
        command += name;
        command += ' ';
        command += s;
        command += "\"";
        p(command);
        command = "set yrange[0:";
        command += to_string(max_ones);
        command += "]";
        p(command);
        command = "plot \"C:/traficoEscom/plots/";
        command += s;
        command += ".txt\" using ($0+1):1 title \"";
        command += s;
        command += "\" pointtype 2 lt rgb \"blue\"";
        p(command);
    }
}

// Ajustar origen de las calles y estacionamientos
void adjust_origin(int x, int y){
    for(auto &r: roads){
        r.x += x;
        r.y += y;
    }
    temp = roads;
    for(auto &p: parking_slots){
        p.x += x;
        p.y += y;
    }
}

// Reiniciar a estado 0
void vaciar_estacionamiento(){
    for(auto &p: parking_slots){
        for(auto &lugar: p.p){
            if(lugar.first > 0)
                lugar.first = 1;
        }
    }
}

int main()
{
    int delay = 55; //Tiempo entre pasos
    int spd = 50; //Velocidad relativa a delay
    bool one_step = false; //Bandera para un paso
    int prob = 30; //Probabilidad para llenado aleatorio
    bool pause = 1; //Bandera para pausas
    bool park = false;
    int horas = 0;
    int minutos = 0;
    int segundos = 0;
    bool prob_switch = false;
    srand(seed);
    sf::Clock delayTimer;

    ofstream densidad_calles;
    ofstream densidad_parking;
    ofstream densidad_total;

    //Cargar texturas
    //Carro debe ver a la derecha para tener la orientación correcta
    sf::Texture carroTexture = loadImage("images/car_test.png", cellsize, cellsize);
    sf::Sprite carro(carroTexture);

    sf::Texture motoTexture = loadImage("images/moto.png", cellsize, cellsize);
    sf::Sprite moto(motoTexture);

    sf::Texture disabledTexture = loadImage("images/disabled.jpg", cellsize*5/4, cellsize);
    sf::Sprite disabled(disabledTexture);

    sf::Texture compassTexture = loadImage("images/compass.png", cellsize*3, cellsize*3);
    sf::Sprite compass(compassTexture);
    compass.setPosition(cellsize * gridx - 3 * cellsize - 5, 40);

    sf::Texture entradaTexture = loadImage("images/entrada.png", cellsize*2, cellsize*2);
    sf::Sprite entrada(entradaTexture);
    entrada.setPosition(cellsize*10, cellsize*16);

    sf::Texture bicisTexture = loadImage("images/bicis.png", cellsize*5, cellsize+8);
    sf::Sprite bicis(bicisTexture);
    bicis.setPosition(cellsize*5, cellsize*17+40-8);

    sf::Texture techoTexture = loadImage("images/techo.png", cellsize*2, cellsize);
    sf::Sprite techo(techoTexture);
    techo.setPosition(cellsize*10, cellsize*17+40);

    sf::Texture pisoTexture = loadImage("images/piso.png", cellsize, cellsize);
    sf::Sprite piso(pisoTexture);

    sf::Texture casetaTexture = loadImage("images/caseta.png", cellsize*2, cellsize*2);
    sf::Sprite caseta(casetaTexture);
    caseta.setPosition(cellsize*12, cellsize*17+(2*cellsize/3)+40);

    sf::Texture pauseTexture = loadImage("images/pause.png", 25, 25);
    sf::Sprite pausa(pauseTexture);
    pausa.setPosition(5,5);

    sf::Texture playTexture = loadImage("images/play.png", 25, 25);
    sf::Sprite play(playTexture);
    play.setPosition(5,5);

    sf::Texture randomTexture = loadImage("images/random.png", 25, 25);
    sf::Sprite aleatorio(randomTexture);
    aleatorio.setPosition(40,5);

    sf::Texture deleteTexture = loadImage("images/delete.png", 25, 25);
    sf::Sprite del(deleteTexture);
    del.setPosition(75,5);

    sf::Texture saveTexture = loadImage("images/save.png", 25, 25);
    sf::Sprite save(saveTexture);
    save.setPosition(110,5);

    sf::Texture loadTexture = loadImage("images/load.png", 25, 25);
    sf::Sprite load(loadTexture);
    load.setPosition(145,5);

    sf::Texture arrowTexture = loadImage("images/arrow.png", 25, 25);
    sf::Sprite arrow(arrowTexture);
    arrow.setPosition(110,5);

    sf::Texture plusTexture = loadImage("images/plus.png", 12, 12);
    sf::Sprite plus_prob(plusTexture);
    plus_prob.setPosition(gridx * cellsize - 195,4);

    sf::Sprite plus_speed(plusTexture);
    plus_speed.setPosition(250,4);

    sf::Sprite plus_turn(plusTexture);
    plus_turn.setPosition(cellsize * gridx - 430, 4);

    sf::Sprite plus_park(plusTexture);
    plus_park.setPosition(cellsize * gridx - 605, 4);

    sf::Sprite plus_stuck(plusTexture);
    plus_stuck.setPosition(cellsize * gridx - 850, 4);

    sf::Texture minusTexture = loadImage("images/minus.png", 12, 12);
    sf::Sprite minus_prob(minusTexture);
    minus_prob.setPosition(gridx * cellsize - 195,20);

    sf::Sprite minus_speed(minusTexture);
    minus_speed.setPosition(250,20);

    sf::Sprite minus_turn(minusTexture);
    minus_turn.setPosition(cellsize * gridx - 430, 20);

    sf::Sprite minus_park(minusTexture);
    minus_park.setPosition(cellsize * gridx - 605, 20);

    sf::Sprite minus_stuck(minusTexture);
    minus_stuck.setPosition(cellsize * gridx - 850, 20);

    sf::RectangleShape caja_prob(sf::Vector2f(26,26));
    caja_prob.setFillColor(sf::Color::Black);
    caja_prob.setOutlineThickness(2);
    caja_prob.setOutlineColor(sf::Color::White);
    caja_prob.setPosition(gridx * cellsize - 238, 7);
    sf::RectangleShape prob_select(sf::Vector2f(18,18));
    prob_select.setFillColor(sf::Color::Black);
    prob_select.setPosition(gridx * cellsize - 234,11);

    sf::RectangleShape caja_trabar(sf::Vector2f(26,26));
    caja_trabar.setFillColor(sf::Color::Black);
    caja_trabar.setOutlineThickness(2);
    caja_trabar.setOutlineColor(sf::Color::White);
    caja_trabar.setPosition(gridx * cellsize - 893, 7);
    sf::RectangleShape trabar_select(sf::Vector2f(18,18));
    trabar_select.setFillColor(sf::Color::White);
    trabar_select.setPosition(gridx * cellsize - 889,11);

    //Textos
    sf::Font font;
    if(!font.loadFromFile("C:\\fonts\\roboto.ttf")){
        cerr << "No font found" << endl;
        return -1;
    }

    //Probabilidad
    sf::Text probability;
    probability.setFont(font);
    probability.setPosition(cellsize * gridx - 175,0);
    probability.setFillColor(sf::Color::White);
    string probabilidad;

    //Velocidad
    sf::Text speed;
    speed.setFont(font);
    speed.setPosition(270,0);
    speed.setFillColor(sf::Color::White);
    string velocidad;

    sf::Text time;
    time.setFont(font);
    time.setPosition(145, 0);
    time.setFillColor(sf::Color::White);
    string hora;

    sf::Text turn;
    turn.setFont(font);
    turn.setPosition(cellsize*gridx - 410, 0);
    turn.setFillColor(sf::Color::White);
    string vuelta;

    sf::Text parking;
    parking.setFont(font);
    parking.setPosition(cellsize*gridx - 585, 0);
    parking.setFillColor(sf::Color::White);
    string estacionamiento;

    sf::Text trabara;
    trabara.setFont(font);
    trabara.setPosition(cellsize*gridx - 830, 0);
    trabara.setFillColor(sf::Color::White);
    string trabado;

    //Fondo
    sf::Color verde_pasto(102,255,102);
    sf::Color verde_oscuro(82,204,82);
    sf::Color ultra_oscuro(22,124,22);
    sf::RectangleShape pasto(sf::Vector2f(gridx*cellsize, gridy*cellsize));
    pasto.setPosition(0, 40);
    pasto.setFillColor(verde_pasto);

    //Mostrar en pantalla
    sf::RenderWindow window(sf::VideoMode(gridx*cellsize, 40 + gridy*cellsize),"Trafico", sf::Style::Default);
    create_roads();
    create_parking();
    adjust_origin(0, 2);

    int max_ones = 0;
    for(auto &r: roads)
        for(auto &space: r.r)
            max_ones++;
    for(auto &p: parking_slots)
        for(auto &s: p.p)
            max_ones++;

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Escape){
                    window.close();
                }
                if(event.key.code == sf::Keyboard::Space){
                    pause = 1 - pause;
                }
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                //Pause / Play
                if(mousePosition.x > 5 and mousePosition.x < 35 and mousePosition.y > 5 and mousePosition.y < 35){
                    pause = 1 - pause;
                }
                //Borrar
                if(del.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    erase();
                    horas = 0;
                    minutos = 0;
                }
                //Aleatorio
                if(aleatorio.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    random_start(horas);
                }
                //Avanzar un paso
                if(arrow.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    one_step = true;
                }
                //Velocidad +
                if(plus_speed.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and spd < 100){
                    delay -= 5;
                    spd += 5;
                }
                //Velocidad -
                if(minus_speed.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and spd > 5){
                    delay += 5;
                    spd -= 5;
                }
                //Probabilidad aparición+
                if(plus_prob.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and prob < 100)
                    prob += 5;
                //Probabilidad aparición-
                if(minus_prob.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and prob > 5){
                    prob -= 5;
                }
                // Probabilidad vuelta+
                if(plus_turn.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and turn_prob < 100)
                    turn_prob += 5;
                //Probabilidad vuelta-
                if(minus_turn.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and turn_prob > 0){
                    turn_prob -= 5;
                }
                // Probabilidad estacionar+
                if(plus_park.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and parking_prob < 100)
                    parking_prob += 5;
                //Probabilidad estacionar-
                if(minus_park.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and parking_prob > 0){
                    parking_prob -= 5;
                }
                // Probabilidad estacionar+
                if(plus_stuck.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and trabar_prob < 9999)
                    trabar_prob += 1;
                //Probabilidad estacionar-
                if(minus_stuck.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and trabar_prob > 0){
                    trabar_prob -= 1;
                }
                if(caja_prob.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    prob_switch = !prob_switch;
                    if(prob_switch)
                        prob_select.setFillColor(sf::Color::White);
                    else
                        prob_select.setFillColor(sf::Color::Black);
                }
                if(caja_trabar.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    trabar_switch = !trabar_switch;
                    if(trabar_switch)
                        trabar_select.setFillColor(sf::Color::White);
                    else
                        trabar_select.setFillColor(sf::Color::Black);
                }
                if(pause){
                    if(mousePosition.y > 35){
                        bool off = false;
                        for (Road &r : roads) {

                            if(toggleCellState(r, mousePosition.x, mousePosition.y - 40, off))
                                off = true;
                        }
                    }
                }
            }
        }

        if(horas >= 18 or horas < 8){
            pasto.setFillColor(ultra_oscuro);
        } else if((horas >= 8 and horas < 10) or (horas >= 16 and horas < 18)){
            pasto.setFillColor(verde_oscuro);
        } else {
            pasto.setFillColor(verde_pasto);
        }

        probabilidad = "Spawn: ";
        if(prob_switch)
            probabilidad += to_string(prob);
        else
            probabilidad += to_string(spawn_prob);
        probabilidad += '%';
        probability.setString(probabilidad);

        velocidad = "Speed: ";
        velocidad += to_string(spd);
        speed.setString(velocidad);

        vuelta = "Turn: ";
        vuelta += to_string(turn_prob);
        vuelta += '%';
        turn.setString(vuelta);

        estacionamiento = "Park: ";
        estacionamiento += to_string(parking_prob);
        estacionamiento += '%';
        parking.setString(estacionamiento);

        trabado = "Stop: ";
        trabado += to_string(trabar_prob);
        trabado += "/19999";
        trabara.setString(trabado);

        hora = "";
        if(horas < 10)
            hora += '0';
        hora += to_string(horas);
        hora += ':';
        if(minutos < 10)
            hora += '0';
        hora += to_string(minutos);
        time.setString(hora);

        window.clear();
        window.draw(pasto);
        for(int i=0; i<gridx; i++){
            for(int j=18; j<gridy; j++){
                piso.setPosition(cellsize*i, cellsize*j+40);
                window.draw(piso);
            }
        }
        window.draw(caseta);
        window.draw(compass);
        window.draw(entrada);
        window.draw(bicis);
        window.draw(techo);
        window.draw(del);
        window.draw(aleatorio);
        //window.draw(save);
        //window.draw(load);
        window.draw(arrow);
        window.draw(speed);
        window.draw(plus_speed);
        window.draw(minus_speed);
        window.draw(probability);
        window.draw(plus_prob);
        window.draw(minus_prob);
        window.draw(time);
        window.draw(caja_prob);
        window.draw(prob_select);
        window.draw(turn);
        window.draw(plus_turn);
        window.draw(minus_turn);
        window.draw(parking);
        window.draw(plus_park);
        window.draw(minus_park);
        window.draw(trabara);
        window.draw(minus_stuck);
        window.draw(plus_stuck);
        window.draw(caja_trabar);
        window.draw(trabar_select);


        //Aplicar la lógica del tráfico
        if ((!pause and delayTimer.getElapsedTime().asMilliseconds() >= delay) or one_step) {
            spawn_prob = probabilidad_por_hora(horas, minutos);
            if(prob_switch)
                spawn(prob);
            else
                spawn(spawn_prob);
            if(!park)
                step(horas);
            if(park)
                estacionar(horas);
            avanzar_tiempo(horas, minutos, segundos);
            park = !park;
            one_step = false;
            delayTimer.restart();
            if(horas == 23 and minutos == 55){
                densidad_calles.close();
                densidad_parking.close();
                densidad_total.close();
                string aux;
                if(prob_switch)
                    aux = to_string(prob);
                else
                    aux = "default";
                plot(max_ones*3/4,aux);
            }

            if(horas == 21 and minutos == 30){
                vaciar_estacionamiento();
            }

            if(horas == 0 and minutos == 5){
                densidad_calles.open("C:\\traficoEscom\\plots\\Calles.txt");
                densidad_parking.open("C:\\traficoEscom\\plots\\Estacionamiento.txt");
                densidad_total.open("C:\\traficoEscom\\plots\\Total.txt");
            }

            int calles, parkings;

            if(minutos % 5 == 0 or (horas == 23 and minutos == 59)){
                calles = parkings = 0;
                for(auto &r: roads)
                    for(auto &space: r.r)
                        if(space > 0) calles++;
                for(auto &p: parking_slots)
                    for(auto &s: p.p)
                        if(s.first > 0) parkings++;
                if(densidad_calles.is_open()){
                    densidad_calles << calles << "\n";
                }
                if(densidad_parking.is_open()) {
                    densidad_parking << parkings << "\n";
                }
                if(densidad_total.is_open()){
                    densidad_total << calles + parkings << "\n";
                }
            }
        }
        if(!pause) window.draw(pausa);
        else window.draw(play);

        for(Road r: roads)
            draw_road(r, window);
        for(Road r: roads)
            draw_cars(r, window, carro, moto);
        for(Parking p: parking_slots)
            draw_parking_slots(p, window, disabled);
        for(Parking p: parking_slots)
            draw_parked_cars(p, window, carro, moto);
        window.display();
    }

    return 0;
}

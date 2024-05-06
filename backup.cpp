#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <set>
#include <functional>
using namespace std;

int seed = time(NULL);
const int gridx = 40;
const int gridy = 15;
const int cellsize = 48;
const vector<bool> reglas = {0,0,0,1,1,1,0,1};
enum Direction {UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3};
enum Type {BLUE = 0, NORMAL = 1, BIG = 2, MOTORCYCLE = 3};
vector<bool> tru = {true};
vector<bool> fols = {false, false};
vector<bool> random_spawn{false};
bool priority = false;

class Road{
public:
    vector<bool> r;
    int x, y;
    vector<bool>::iterator prev, next;
    map<int, vector<bool>::iterator> intersections;
    set<int> turns, empty_turns;
    Direction direction;
    Road(int x, int y, int size, Direction dir, vector<bool>::iterator prev = fols.begin(), vector<bool>::iterator next = tru.begin())
    : x(x), y(y), direction(dir), prev(prev), next(next){
        r.resize(size, 0);
    }
};
vector<Road> roads;
vector<Road> temp;

class Parking{
public:
    vector<pair<int, vector<bool>::iterator>> p;
    Direction direction, drawdirection;
    Type type;
    int x,y;

    Parking(int x, int y, int size, Direction dir, Direction draw, Type type, Road &r, int start)
    : x(x), y(y), direction(dir), drawdirection(draw), type(type){
        p.resize(size);
        for(int i = 0; i < size; i++)
            p[i] = {0, r.r.begin() + start + i};
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
void turn_cars(Road &road, int prob){
    for(auto turn: road.turns){
        if(road.r[turn] and
        // Si es el fin de la calle está forzado a dar vuelta
          (rand() % 100 < prob or (road.r.begin() + turn + 1) == road.r.end())){
            road.r[turn] = false;
            *road.intersections[turn - 1] = true;
        }
    }
}

//Inicializar las calles
void create_roads(){
    roads.push_back(Road(10, 1, 11, DOWN));
    roads.push_back(Road(18, 3, 9, DOWN));
    roads.push_back(Road(27, 11, 11, UP));

    roads.push_back(Road(35,3, 34, LEFT));
    roads.push_back(Road(2,4, 7, DOWN));
    roads.push_back(Road(2,11,34,RIGHT));
    roads.push_back(Road(35,10,7,UP));

    roads[0].prev = random_spawn.begin();               // Inicio
    roads[2].next = fols.begin();                       // Fin

    join_roads(roads[3], roads[4]);
    join_roads(roads[4], roads[5]);
    join_roads(roads[5], roads[6]);
    join_roads(roads[6], roads[3]);

    intersect_roads(roads[0], 2, roads[3], 25);
    intersect_roads(roads[0], 10, roads[5], 8);
    intersect_roads(roads[1], 0, roads[3], 17);
    intersect_roads(roads[1], 8, roads[5], 16);
    intersect_roads(roads[2], 8, roads[3], 8);
    intersect_roads(roads[2], 0, roads[5], 25);

    roads[0].turns = {2, 10};
    roads[1].turns = {8};
    roads[2].turns = {8};
    roads[3].turns = {8, 17, 25};
    roads[4].turns = {};
    roads[5].turns = {25};
    roads[6].turns = {};

    temp = roads;
}

//Inicializar los lugares de estacionamiento
void create_parking(){
    parking_slots.push_back(Parking(4, 10, 4, UP, RIGHT, BLUE, roads[5], 2));
    parking_slots.push_back(Parking(4, 12, 4, DOWN, RIGHT, BLUE, roads[5], 2));
    parking_slots.push_back(Parking(12, 10, 4, UP, RIGHT, BIG, roads[5], 10));
    parking_slots.push_back(Parking(13, 12, 4, DOWN, RIGHT, BIG, roads[5], 11));
    parking_slots.push_back(Parking(18,12,17,DOWN,RIGHT,NORMAL,roads[5],16));
    parking_slots.push_back(Parking(9,2,7,UP,LEFT,NORMAL,roads[3], 26));
    parking_slots.push_back(Parking(1,4,5,LEFT,DOWN,NORMAL,roads[4],0));
    parking_slots.push_back(Parking(1,3,1,LEFT,DOWN,NORMAL,roads[3],33));
    parking_slots.push_back(Parking(8,4,5,DOWN,LEFT,NORMAL,roads[3],27));
    parking_slots.push_back(Parking(3,5,5,RIGHT,DOWN,NORMAL,roads[4],2));
    parking_slots.push_back(Parking(9,5,5,LEFT,DOWN,NORMAL,roads[0],4));
    parking_slots.push_back(Parking(25,2,14,UP,LEFT,NORMAL,roads[3],10));
    parking_slots.push_back(Parking(16,4,4,DOWN,LEFT,BIG,roads[3],19));
    parking_slots.push_back(Parking(11,5,5,RIGHT,DOWN,NORMAL,roads[0],4));
    parking_slots.push_back(Parking(17,5,5,LEFT,DOWN,NORMAL,roads[1],2));
    parking_slots.push_back(Parking(25,4,6,DOWN,LEFT,NORMAL,roads[3],10));
    parking_slots.push_back(Parking(19,5,5,RIGHT,DOWN,NORMAL,roads[1],2));
    parking_slots.push_back(Parking(20,10,6,UP,RIGHT,NORMAL,roads[5],18));
    parking_slots.push_back(Parking(26,9,5,LEFT,UP,NORMAL,roads[2],2));
    parking_slots.push_back(Parking(28,9,5,RIGHT,UP,NORMAL,roads[2],2));
    parking_slots.push_back(Parking(29,10,5,UP,RIGHT,NORMAL,roads[5],27));
    parking_slots.push_back(Parking(34,9,5,LEFT,UP,NORMAL,roads[6],1));
    parking_slots.push_back(Parking(33,4,5,DOWN,LEFT,NORMAL,roads[3],2));
    parking_slots.push_back(Parking(35,2,7,UP,LEFT,NORMAL,roads[3],0));
    parking_slots.push_back(Parking(36,10,7,RIGHT,UP,NORMAL,roads[6],0));
}

//Calcular regla según la vecindad
int regla_actual(int cell, Road &r){
    int resultado = 0;

    //Checar la continuación de otra calle anterior
    if(cell-1 < 0)
        resultado += 4 *  *r.prev;
    else
        resultado += 4 * r.r[cell-1];

    resultado += 2 * r.r[cell];

    if(cell+1 > r.r.size()-1)
        resultado += *r.next;
    else
        resultado += r.r[cell+1];
    return resultado;
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
    if(road.intersections.find(cell) != road.intersections.end() and road.r[cell]){
        //Si hay alguien en frente, estorbando, no hacer nada
        if(*road.intersections[cell]){
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
int tiempo_a_estacionar(){
    int n = rand() % 100;
    if(n < 5) return 1;
    else if(n < 10) return 2;
    else if(n < 15) return 3;
    else if(n < 25) return 4;
    else if(n < 40) return 5;
    else if(n < 55) return 6;
    else if(n < 70) return 7;
    else if(n < 80) return 8;
    else if(n < 85) return 9;
    else if(n < 90) return 10;
    else if(n < 95) return 11;
    return 12;
}

//Manejar el estacionamiento de los carros
void estacionar(){
    int prob = 80;
    for(Parking &p: parking_slots) {
        for (auto &lugar: p.p) {
            if (lugar.first <= 0 and *lugar.second) {
                if (rand() % 100 < prob) {
                    lugar.first = tiempo_a_estacionar() * 60;
                    *lugar.second = false;
                }
            } else if (lugar.first == 1 and !*lugar.second) {
                lugar.first = 0;
                *lugar.second = true;
            } else if (lugar.first > 1) {
                lugar.first--;
            }
        }
    }
}

//Aplicar la regla 186 para avanzar a los carros
void step(int turn_prob){
    for(int i=0; i<roads.size(); i++) {
        for (int cell = 0; cell < roads[i].r.size(); cell++){
            //Si hay un cruce, checar si se puede avanzar, sino, no avanzar
            if(!cruce(roads[i], cell))
                temp[i].r[cell] = reglas[regla_actual(cell, roads[i])];
            else {
                temp[i].r[cell] = roads[i].r[cell];
                roads[i].r[cell] = false;
            }
        }
    }

    //Aplicar cambios
    roads = temp;

    //Dar vueltas
    for(auto &r: roads)
        turn_cars(r, turn_prob);
}

//Inicializar calles aleatoriamente
void random_start(int prob){
    for(auto &r: roads)
        for (int j = 0; j < r.r.size(); j++)
            r.r[j] = rand() % 100 < prob;
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
void draw_parked_cars(Parking &p, sf::RenderWindow &window, sf::Sprite &sprite){
    sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Sprite car = sprite;
    if(p.direction == LEFT){
        car.setOrigin(bounds.width, bounds.height);
        car.rotate(180.0f);
    } else if (p.direction == DOWN){
        car.setOrigin(bounds.width - cellsize, bounds.height);
        car.rotate(90.0f);
    } else if(p.direction == UP){
        car.setOrigin(bounds.width, bounds.height - cellsize);
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

//Dibujar carros
void draw_cars(Road &r, sf::RenderWindow &window, sf::Sprite &sprite){
    sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Sprite up, down, left;
    up = sprite;
    up.setOrigin(bounds.width, bounds.height - cellsize);
    up.rotate(270.0f);
    down = sprite;
    down.setOrigin(bounds.width - cellsize, bounds.height);
    down.rotate(90.0f);
    left = sprite;
    left.setOrigin(bounds.width, bounds.height);
    left.rotate(180.0f);
    if(r.direction == UP){
        for(int i = r.y, j = 0; j < r.r.size(); i--, j++){
            if(r.r[j]){
                up.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(up);
            }
        }
    } else if(r.direction == RIGHT){
        for(int i = r.x, j = 0; j < r.r.size(); i++, j++){
            if(r.r[j]){
                sprite.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(sprite);
            }
        }
    } else if(r.direction == DOWN){
        for(int i = r.y, j = 0; j < r.r.size(); i++, j++){
            if(r.r[j]){
                down.setPosition(r.x * cellsize, 40 + i * cellsize);
                window.draw(down);
            }
        }
    } else if(r.direction == LEFT) {
        for (int i = r.x, j = 0; j < r.r.size(); i--, j++) {
            if (r.r[j]) {
                left.setPosition(i * cellsize, 40 + r.y * cellsize);
                window.draw(left);
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

void erase(){
    for (Road &r : roads)
        fill(r.r.begin(), r.r.end(), false);
    for (Parking &p: parking_slots)
        for(auto &s: p.p)
            s.first = 0;
}

//Cargar y ajustar tamaño de imagen
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

int __main()
{
    int delay = 255; //Tiempo entre pasos
    int spd = 50; //Velocidad relativa a delay
    bool one_step = false; //Bandera para un paso
    const int turn_prob = 33;
    const int spawn_prob = 50;
    int prob = 30; //Probabilidad para llenado aleatorio
    bool pause = 1; //Bandera para pausas
    bool park = false;
    srand(seed);
    sf::Clock delayTimer;

    //Cargar texturas
    //Carro debe ver a la derecha para tener la orientación correcta
    sf::Texture carroTexture = loadImage("images/car_test.png", cellsize, cellsize);
    sf::Sprite carro(carroTexture);

    sf::Texture disabledTexture = loadImage("images/disabled.jpg", cellsize*5/4, cellsize);
    sf::Sprite disabled(disabledTexture);

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
    plus_prob.setPosition(gridx * cellsize - 170,4);

    sf::Sprite plus_speed(plusTexture);
    plus_speed.setPosition(gridx * cellsize / 2 - 110,4);

    sf::Texture minusTexture = loadImage("images/minus.png", 12, 12);
    sf::Sprite minus_prob(minusTexture);
    minus_prob.setPosition(gridx * cellsize - 170,20);

    sf::Sprite minus_speed(minusTexture);
    minus_speed.setPosition(gridx * cellsize / 2 - 110,20);

    //Textos
    sf::Font font;
    if(!font.loadFromFile("C:\\fonts\\roboto.ttf")){
        cerr << "No font found" << endl;
        return -1;
    }

    //Probabilidad
    sf::Text probability;
    probability.setFont(font);
    probability.setPosition(cellsize * gridx -150,0);
    probability.setFillColor(sf::Color::White);
    string probabilidad;

    //Velocidad
    sf::Text speed;
    speed.setFont(font);
    speed.setPosition(gridx * cellsize / 2 - 90,0);
    speed.setFillColor(sf::Color::White);
    string velocidad;

    //Fondo
    sf::Color verde_pasto(102,255,102);
    sf::RectangleShape pasto(sf::Vector2f(gridx*cellsize, gridy*cellsize));
    pasto.setPosition(0, 40);
    pasto.setFillColor(verde_pasto);

    //Mostrar en pantalla
    sf::RenderWindow window(sf::VideoMode(gridx*cellsize, 40 + gridy*cellsize),"Trafico", sf::Style::Default);
    create_roads();
    create_parking();
    random_start(prob);
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
                }
                //Aleatorio
                if(aleatorio.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    random_start(prob);
                }
                //Avanzar un paso
                if(arrow.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y))){
                    one_step = true;
                }
                //Velocidad +
                if(plus_speed.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and spd < 100){
                    delay -= 25;
                    spd += 5;
                }
                //Velocidad -
                if(minus_speed.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and spd > 5){
                    delay += 25;
                    spd -= 5;
                }
                //Probabilidad +
                if(plus_prob.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and prob < 100)
                    prob += 5;
                //Probabilidad -
                if(minus_prob.getGlobalBounds().contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)) and prob > 5){
                    prob -= 5;
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

        probabilidad = "Prob: ";
        probabilidad += to_string(prob);
        probability.setString(probabilidad);

        velocidad = "Speed: ";
        velocidad += to_string(spd);
        speed.setString(velocidad);

        window.clear();
        window.draw(pasto);
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

        //Aplicar la lógica del tráfico
        if ((!pause and delayTimer.getElapsedTime().asMilliseconds() >= delay) or one_step) {
            random_spawn[0] = (rand() % 100) < spawn_prob;
            if(!park)
                step(turn_prob);
            if(park)
                estacionar();
            park = !park;
            one_step = false;
            delayTimer.restart();
        }
        if(!pause) window.draw(pausa);
        else window.draw(play);

        for(Road r: roads)
            draw_road(r, window);
        for(Road r: roads)
            draw_cars(r, window, carro);
        for(Parking p: parking_slots)
            draw_parking_slots(p, window, disabled);
        for(Parking p: parking_slots)
            draw_parked_cars(p, window, carro);
        window.display();
    }

    return 0;
}

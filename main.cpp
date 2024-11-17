#include <SFML/Graphics.hpp>
#include <queue>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

const int GRID_SIZE = 25;     // Ukuran grid (100x100)
const int CELL_SIZE = 16;       // Ukuran setiap sel dalam piksel
const int WINDOW_SIZE = GRID_SIZE * CELL_SIZE;

struct Node {
    int x, y;
    float g, h;
    Node* parent;

    Node(int x, int y, float g = 0, float h = 0, Node* parent = nullptr) 
        : x(x), y(y), g(g), h(h), parent(parent) {}

    float f() const { return g + h; }

    bool operator<(const Node& other) const { return f() > other.f(); }
};

vector<Node> getNeighbors(const Node& current, const vector<vector<int>>& grid) {
    vector<Node> neighbors;
    int x = current.x, y = current.y;
    vector<pair<int, int>> directions = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };

    for (auto& d : directions) {
        int nx = x + d.first, ny = y + d.second;
        if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && grid[ny][nx] == 0) {
            neighbors.emplace_back(nx, ny);
        }
    }
    return neighbors;
}

float heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2); // Manhattan distance
}

void reconstructPath(Node* node, vector<vector<int>>& grid) {
    while (node) {
        grid[node->y][node->x] = 2; // Mark path
        node = node->parent;
    }
}

bool aStarStepByStep(sf::RenderWindow& window, const sf::Vector2i& start, const sf::Vector2i& goal, vector<vector<int>>& grid) {
    if (start.x == -1 || start.y == -1 || goal.x == -1 || goal.y == -1) {
        cout << "Start and Goal must be set before running the algorithm!\n";
        return false;
    }

    priority_queue<Node> openList;
    openList.emplace(start.x, start.y, 0, heuristic(start.x, start.y, goal.x, goal.y));
    vector<vector<bool>> closedList(GRID_SIZE, vector<bool>(GRID_SIZE, false));

    while (!openList.empty()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            cout << "Process interrupted by user.\n";
            return false;
        }

        Node current = openList.top();
        openList.pop();

        if (closedList[current.y][current.x]) continue;
        closedList[current.y][current.x] = true;

        if (current.x == goal.x && current.y == goal.y) {
            reconstructPath(&current, grid);
            return true;
        }

        for (auto& neighbor : getNeighbors(current, grid)) {
            if (!closedList[neighbor.y][neighbor.x]) {
                neighbor.g = current.g + 1;
                neighbor.h = heuristic(neighbor.x, neighbor.y, goal.x, goal.y);
                neighbor.parent = new Node(current); // Make sure parent is valid
                openList.push(neighbor);
                grid[neighbor.y][neighbor.x] = 3; // Mark as visited
            }
        }

        window.clear();
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);

                if (grid[y][x] == 1) cell.setFillColor(sf::Color::Black);         // Rintangan
                else if (grid[y][x] == 2) cell.setFillColor(sf::Color::Blue);    // Jalur
                else if (grid[y][x] == 3) cell.setFillColor(sf::Color::Red);     // Dikunjungi
                else if (x == start.x && y == start.y) cell.setFillColor(sf::Color::Green); // Start
                else if (x == goal.x && y == goal.y) cell.setFillColor(sf::Color::Yellow);  // Goal
                else cell.setFillColor(sf::Color::White);                        // Area bebas

                window.draw(cell);
            }
        }
        window.display();
        this_thread::sleep_for(chrono::milliseconds(50)); // Delay 50ms
    }

    return false;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "A* Pathfinding - Draw Obstacles");
    window.setFramerateLimit(60);

    vector<vector<int>> grid(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    sf::Vector2i start(-1, -1), goal(-1, -1);

    bool running = false;
    bool drawingObstacles = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Klik kiri untuk memilih start dan end
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int x = event.mouseButton.x / CELL_SIZE;
                int y = event.mouseButton.y / CELL_SIZE;
                if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE && grid[y][x] == 0) {
                    if (start.x == -1 && start.y == -1) {
                        start = sf::Vector2i(x, y);
                        cout << "Start set at: (" << x << ", " << y << ")\n";
                    } else if (goal.x == -1 && goal.y == -1) {
                        goal = sf::Vector2i(x, y);
                        cout << "Goal set at: (" << x << ", " << y << ")\n";
                        running = true;
                    }
                }
            }

            // Klik kanan untuk mulai menggambar rintangan
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                drawingObstacles = true;
            }

            // Lepaskan klik kanan untuk berhenti menggambar rintangan
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                drawingObstacles = false;
            }
        }

        // Jika menggambar rintangan
        if (drawingObstacles) {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            int x = mousePosition.x / CELL_SIZE;
            int y = mousePosition.y / CELL_SIZE;
            if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                grid[y][x] = 1; // Tandai sebagai rintangan
            }
        }

        // Jalankan algoritma setelah start dan goal ditentukan
        if (running) {
            if (!aStarStepByStep(window, start, goal, grid)) {
                cout << "Pathfinding interrupted or completed.\n";
            }
            running = false;
        }

        // Render grid
        window.clear();
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);

                if (grid[y][x] == 1) cell.setFillColor(sf::Color::Black);         // Rintangan
                else if (grid[y][x] == 2) cell.setFillColor(sf::Color::Blue);    // Jalur
                else if (grid[y][x] == 3) cell.setFillColor(sf::Color::Red);     // Dikunjungi
                else if (x == start.x && y == start.y) cell.setFillColor(sf::Color::Green); // Start
                else if (x == goal.x && y == goal.y) cell.setFillColor(sf::Color::Yellow);  // Goal
                else cell.setFillColor(sf::Color::White);                        // Area bebas

                window.draw(cell);
            }
        }
        window.display();
    }

    return 0;
}


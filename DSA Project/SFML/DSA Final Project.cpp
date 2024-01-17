#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;
using namespace std;

// User-defined simple queue
template <typename T>
class Queue {
public:
    void enqueue(const T& value) {
        elements.push_back(value);
    }

    void dequeue() {
        if (!empty()) {
            elements.erase(elements.begin());
        }
    }

    T front() const {
        if (!empty()) {
            return elements[0];
        }
        // Return a default value or throw an exception for an empty queue
        return T();
    }

    bool empty() const {
        return elements.empty();
    }

    size_t size() const {
        return elements.size();
    }

private:
    vector<T> elements;
};

Font font;
class Button {
public:
    RectangleShape shape;
    Text text;
    bool selected;
    Color originalColor;

    Button(float x, float y, float width, float height, Color color, const string& buttonText)
        : shape(Vector2f(width, height)), text(), selected(false) ,originalColor(color) {
        shape.setPosition(x, y);
        shape.setFillColor(color);
        shape.setOutlineColor(Color::Black);
        shape.setOutlineThickness(5);

        text.setFont(font);  // Assuming you have a global font defined
        text.setString(buttonText);
        text.setCharacterSize(32);
        text.setFillColor(Color::Black);
        float textX = x - 5 + (width - text.getLocalBounds().width) / 2;
        float textY = y + (height - text.getLocalBounds().height) / 2 - 5;
        text.setPosition(textX, textY);
        updateColor();
    }
    void setSelected(bool isSelected) {
        selected = isSelected;
        updateColor();
    }

    bool isClicked(float mouseX, float mouseY) const {
        return shape.getGlobalBounds().contains(mouseX, mouseY);
    }
    void updateColor() {
        if (selected) {
            shape.setFillColor(Color(255, 240, 52)); // Change color when selected
        }
        else {
            shape.setFillColor(originalColor); // Use the original color when not selected
        }
    }
};

const int gridSizeX = 32;
const int gridSizeY = 22;
const int windowWidth = 1366;
const int windowHeight = 768;

enum NodeType {
    EMPTY,
    OBSTACLE,
    START,
    END,
    PATH
};

struct Node {
    int x;
    int y;
    NodeType type;
    vector<Node*> neighbors;
    Node* previous;
    int distance;

    Node(int x, int y, NodeType type) : x(x), y(y), type(type), previous(nullptr), distance(numeric_limits<int>::max()) {}
};

vector<vector<Node>> graph(gridSizeX, vector<Node>(gridSizeY, Node(0, 0, EMPTY)));

Node* startNode = nullptr;
Node* endNode = nullptr;

void clearDistances() {
    for (auto& row : graph) {
        for (auto& node : row) {
            node.distance = numeric_limits<int>::max();
            node.previous = nullptr;
        }
    }
}

void addEdge(Node* node1, Node* node2) {
    node1->neighbors.push_back(node2);
    node2->neighbors.push_back(node1);
}

void buildGraph() {
    for (int i = 0; i < gridSizeX; ++i) {
        for (int j = 0; j < gridSizeY; ++j) {
            graph[i][j] = Node(i, j, EMPTY);
        }
    }

    for (int i = 0; i < gridSizeX; ++i) {
        for (int j = 0; j < gridSizeY; ++j) {
            if (i > 0) addEdge(&graph[i][j], &graph[i - 1][j]); // Left neighbor
            if (i < gridSizeX - 1) addEdge(&graph[i][j], &graph[i + 1][j]); // Right neighbor
            if (j > 0) addEdge(&graph[i][j], &graph[i][j - 1]); // Up neighbor
            if (j < gridSizeY - 1) addEdge(&graph[i][j], &graph[i][j + 1]); // Down neighbor
        }
    }
}

void generateMaze() {
    for (int i = 0; i < gridSizeX; i += 2) {
        for (int j = 0; j < gridSizeY; j += 2) {
            graph[i][j].type = OBSTACLE;

            int randomNeighbor = rand() % 4;
            switch (randomNeighbor) {
            case 0: // Up
                if (j > 0) graph[i][j - 1].type = OBSTACLE;
                break;
            case 1: // Right
                if (i < gridSizeX - 1) graph[i + 1][j].type = OBSTACLE;
                break;
            case 2: // Down
                if (j < gridSizeY - 1) graph[i][j + 1].type = OBSTACLE;
                break;
            case 3: // Left
                if (i > 0) graph[i - 1][j].type = OBSTACLE;
                break;
            }
        }
    }
}

bool isUnreachable(Node* start, Node* end) {
    Queue<Node*> q;
    q.enqueue(start);

    start->distance = 0;

    while (!q.empty()) {
        Node* current = q.front();
        q.dequeue();

        for (Node* neighbor : current->neighbors) {
            if (neighbor->type != OBSTACLE && neighbor->distance == numeric_limits<int>::max()) {
                neighbor->distance = current->distance + 1;
                q.enqueue(neighbor);

                if (neighbor == end) {
                    // Found the end node, stop the BFS
                    clearDistances();
                    return false;
                }
            }
        }
    }

    clearDistances();
    return true;
}

void findShortestPath() {
    if (!startNode || !endNode) {
        cout << "Please set both start and end points before finding the path.\n";
        return;
    }

    if (isUnreachable(startNode, endNode) || isUnreachable(endNode, startNode)) {
        cout << "The start or end point is unreachable.\n";
        return;
    }

    Queue<Node*> q;
    q.enqueue(startNode);

    startNode->distance = 0;

    while (!q.empty()) {
        Node* current = q.front();
        q.dequeue();

        for (Node* neighbor : current->neighbors) {
            if (neighbor->type != OBSTACLE && neighbor->distance == numeric_limits<int>::max()) {
                neighbor->distance = current->distance + 1;
                neighbor->previous = current;
                q.enqueue(neighbor);

                if (neighbor == endNode) {
                    // Found the end node, stop the BFS
                    break;
                }
            }
        }
    }

    Node* current = endNode;
    while (current && current != startNode) {
        current->type = PATH;
        current = current->previous;
    }

    clearDistances();
}
void updateButtonSelection(float mouseX, float mouseY, Button& button) {
    button.setSelected(button.isClicked(mouseX, mouseY));
}
// SFML window and event handling
int main() {


    RenderWindow window(VideoMode(windowWidth, windowHeight), "Square Grid", Style::Fullscreen);


    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font file." << endl;
        return 1;
    }

    const float blockSizeX = 30;
    const float blockSizeY = 30;

    buildGraph();

    bool addingStart = false;
    bool addingEnd = false;
    bool addingObstacle = false;

    Button startButton(50, 150, 150, 50, Color(184, 189, 181), "Start");
    Button endButton(50, 250, 150, 50, Color(184, 189, 181), "End");
    Button findPathButton(50, 350, 150, 50, Color(184, 189, 181), "Find Path");
    Button mazeButton(50, 450, 250, 50, Color(184, 189, 181), "Generate Maze");
    Button exitButton(50, 550, 150, 50, Color(184, 189, 181), "Exit");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);
                        // Check if the mouse click is on any button
                updateButtonSelection(mouseX, mouseY, startButton);
                updateButtonSelection(mouseX, mouseY, endButton);
                updateButtonSelection(mouseX, mouseY, findPathButton);
                updateButtonSelection(mouseX, mouseY, mazeButton);
                updateButtonSelection(mouseX, mouseY, exitButton);
                if (startButton.isClicked(mouseX, mouseY)) {
                    addingStart = true;
                    addingEnd = false;
                    addingObstacle = false;
                }
                else if (endButton.isClicked(mouseX, mouseY)) {
                    addingStart = false;
                    addingEnd = true;
                    addingObstacle = false;
                }
                else if (findPathButton.isClicked(mouseX, mouseY)) {
                    findShortestPath();
                }
                else if (mazeButton.isClicked(mouseX, mouseY)) {
                    buildGraph();
                    generateMaze();
                    startNode = nullptr;
                    endNode = nullptr;
                    addingStart = false;
                    addingEnd = false;
                    addingObstacle = false;
                    cout << "Maze generated. You can now set start and end points.\n";
                }
                else if (exitButton.isClicked(mouseX, mouseY)) {
                    window.close();
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                float mouseX = static_cast<float>(event.mouseButton.x);
                float mouseY = static_cast<float>(event.mouseButton.y);

                for (int i = 0; i < gridSizeX; ++i) {
                    for (int j = 0; j < gridSizeY; ++j) {
                        FloatRect blockBounds(350 + i * blockSizeX, 50 + j * blockSizeY, blockSizeX, blockSizeY);
                        if (blockBounds.contains(mouseX, mouseY)) {
                            if (event.mouseButton.button == sf::Mouse::Left) {
                                if (addingStart && !startNode) {
                                    startNode = &graph[i][j];
                                    startNode->type = START;
                                }
                                else if (addingEnd && !endNode && &graph[i][j] != startNode) {
                                    endNode = &graph[i][j];
                                    endNode->type = END;
                                }
                                else if (addingObstacle && graph[i][j].type != START && graph[i][j].type != END) {
                                    graph[i][j].type = OBSTACLE;
                                }
                            }
                        }
                    }
                }
            }

        }
        window.clear(Color(45, 39, 39));

        // Draw buttons
        window.draw(startButton.shape);
        window.draw(startButton.text);
        window.draw(endButton.shape);
        window.draw(endButton.text);
        window.draw(findPathButton.shape);
        window.draw(findPathButton.text);
        window.draw(mazeButton.shape);
        window.draw(mazeButton.text);
        window.draw(exitButton.shape);
        window.draw(exitButton.text);

        // Draw the grid with updated colors
        for (int i = 0; i < gridSizeX; ++i) {
            for (int j = 0; j < gridSizeY; ++j) {
                RectangleShape block(Vector2f(blockSizeX, blockSizeY));
                block.setPosition(350 + i * blockSizeX, 50 + j * blockSizeY);
                block.setOutlineColor(Color::Black);
                block.setOutlineThickness(2);
                Color blockColor;
                switch (graph[i][j].type) {
                case EMPTY:
                    blockColor = Color(184, 189, 181); // Default color
                    break;
                case OBSTACLE:
                    blockColor = Color(0x12, 0x13, 0x0F);
                    break;
                case START:
                    blockColor = Color::Green;
                    break;
                case END:
                    blockColor = Color::Red;
                    break;
                case PATH:
                    blockColor = Color(248, 255, 28); // Color for the path
                    break;
                default:
                    blockColor = Color::White; // Default color
                    break;
                }
                block.setFillColor(blockColor);
                window.draw(block);
            }
        }

        window.display();
    }
   

    return 0;
}
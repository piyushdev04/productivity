#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <unordered_map>

class TaskManager {
private:
    static int getPriorityRank(const std::string& priority) {
        static const std::unordered_map<std::string, int> priorityRank = {
            {"High", 3}, {"Medium", 2}, {"Low", 1}
        };
        auto it = priorityRank.find(priority);
        return (it != priorityRank.end()) ? it->second : 1;
    }

    struct Task {
        std::string description;
        std::string priority;
        bool isCompleted;
        int id;

        static bool comparePriority(const Task& a, const Task& b) {
            return getPriorityRank(a.priority) > getPriorityRank(b.priority);
        }

        std::string toFileString() const {
            return description + "|" + priority + "|" + 
                   (isCompleted ? "1" : "0") + "|" + std::to_string(id);
        }

        static Task fromFileString(const std::string& line) {
            Task task;
            std::istringstream ss(line);
            std::string completed;
            
            std::getline(ss, task.description, '|');
            std::getline(ss, task.priority, '|');
            std::getline(ss, completed, '|');
            std::string idStr;
            std::getline(ss, idStr);

            task.isCompleted = (completed == "1");
            task.id = std::stoi(idStr);
            return task;
        }
    };

    std::vector<Task> tasks;
    std::string filename;
    int nextTaskId;

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void pauseAndClear() {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        clearScreen();
    }

    void displayHeader() {
        clearScreen();
        std::cout << "===== Task Management System =====\n";
        std::cout << "==================================\n\n";
    }

    void saveToFile() {
        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Error: Unable to open file for writing.\n";
            return;
        }

        for (const auto& task : tasks) {
            file << task.toFileString() << std::endl;
        }
        file.close();
    }

    void loadFromFile() {
        std::ifstream file(filename);
        if (!file) {
            return;
        }

        tasks.clear();
        std::string line;
        nextTaskId = 1;
        while (std::getline(file, line)) {
            Task task = Task::fromFileString(line);
            tasks.push_back(task);
            nextTaskId = std::max(nextTaskId, task.id + 1);
        }
        file.close();
    }

    void displayTasks(bool showCompleted = true) {
        if (tasks.empty()) {
            std::cout << "No tasks found.\n";
            return;
        }

        auto sortedTasks = tasks;
        std::sort(sortedTasks.begin(), sortedTasks.end(), Task::comparePriority);

        std::cout << std::left 
                  << std::setw(5) << "ID" 
                  << std::setw(40) << "Description" 
                  << std::setw(15) << "Priority" 
                  << "Status\n";
        std::cout << std::string(70, '-') << "\n";

        for (const auto& task : sortedTasks) {
            if (!showCompleted && task.isCompleted) continue;

            std::cout << std::left 
                      << std::setw(5) << task.id
                      << std::setw(40) << (task.description.length() > 40 ? 
                         task.description.substr(0, 37) + "..." : task.description)
                      << std::setw(15) << task.priority
                      << (task.isCompleted ? "Completed" : "Pending") << "\n";
        }
    }

    bool validatePriority(const std::string& priority) {
        return priority == "High" || priority == "Medium" || priority == "Low";
    }

public:
    TaskManager(const std::string& file = "tasks.txt") : filename(file), nextTaskId(1) {
        loadFromFile();
    }

    void run() {
        while (true) {
            displayHeader();
            std::cout << "1. Add Task\n";
            std::cout << "2. View All Tasks\n";
            std::cout << "3. View Pending Tasks\n";
            std::cout << "4. Edit Task\n";
            std::cout << "5. Mark Task as Completed\n";
            std::cout << "6. Delete Task\n";
            std::cout << "7. Save and Exit\n";
            std::cout << "\nEnter your choice: ";

            int choice;
            std::cin >> choice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (choice) {
                case 1: addTask(); break;
                case 2: 
                    displayHeader();
                    displayTasks(); 
                    pauseAndClear(); 
                    break;
                case 3: 
                    displayHeader();
                    displayTasks(false); 
                    pauseAndClear(); 
                    break;
                case 4: editTask(); break;
                case 5: markTaskCompleted(); break;
                case 6: deleteTask(); break;
                case 7: 
                    saveToFile();
                    std::cout << "Tasks saved. Goodbye!\n";
                    return;
                default: 
                    std::cout << "Invalid choice. Please try again.\n";
                    pauseAndClear();
            }
        }
    }

    void addTask() {
        displayHeader();
        Task newTask;
        
        std::cout << "Enter task description: ";
        std::getline(std::cin, newTask.description);
        
        while (newTask.description.empty()) {
            std::cout << "Description cannot be empty. Try again: ";
            std::getline(std::cin, newTask.description);
        }

        while (true) {
            std::cout << "Enter task priority (High/Medium/Low): ";
            std::getline(std::cin, newTask.priority);
            
            if (validatePriority(newTask.priority)) {
                break;
            }
            std::cout << "Invalid priority. Please choose High, Medium, or Low.\n";
        }

        newTask.isCompleted = false;
        newTask.id = nextTaskId++;

        tasks.push_back(newTask);
        std::cout << "Task added successfully!\n";
        pauseAndClear();
    }

    void editTask() {
        displayHeader();
        displayTasks();

        if (tasks.empty()) {
            pauseAndClear();
            return;
        }

        int taskId;
        std::cout << "Enter task ID to edit: ";
        std::cin >> taskId;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto it = std::find_if(tasks.begin(), tasks.end(), 
            [taskId](const Task& t) { return t.id == taskId; });

        if (it != tasks.end()) {
            std::cout << "Current description: " << it->description << "\n";
            std::cout << "Enter new description (or press Enter to keep current): ";
            std::string newDesc;
            std::getline(std::cin, newDesc);
            if (!newDesc.empty()) {
                it->description = newDesc;
            }

            while (true) {
                std::cout << "Current priority: " << it->priority << "\n";
                std::cout << "Enter new priority (High/Medium/Low, or press Enter to keep current): ";
                std::string newPriority;
                std::getline(std::cin, newPriority);
                
                if (newPriority.empty()) break;
                
                if (validatePriority(newPriority)) {
                    it->priority = newPriority;
                    break;
                }
                std::cout << "Invalid priority. Please choose High, Medium, or Low.\n";
            }

            std::cout << "Task updated successfully!\n";
        } else {
            std::cout << "Task not found.\n";
        }
        pauseAndClear();
    }

    void markTaskCompleted() {
        displayHeader();
        displayTasks(false);

        if (tasks.empty()) {
            pauseAndClear();
            return;
        }

        int taskId;
        std::cout << "Enter task ID to mark as completed: ";
        std::cin >> taskId;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto it = std::find_if(tasks.begin(), tasks.end(), 
            [taskId](const Task& t) { return t.id == taskId; });

        if (it != tasks.end()) {
            it->isCompleted = true;
            std::cout << "Task marked as completed!\n";
        } else {
            std::cout << "Task not found.\n";
        }
        pauseAndClear();
    }

    void deleteTask() {
        displayHeader();
        displayTasks();

        if (tasks.empty()) {
            pauseAndClear();
            return;
        }

        int taskId;
        std::cout << "Enter task ID to delete: ";
        std::cin >> taskId;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto it = std::find_if(tasks.begin(), tasks.end(), 
            [taskId](const Task& t) { return t.id == taskId; });

        if (it != tasks.end()) {
            tasks.erase(it);
            std::cout << "Task deleted successfully!\n";
        } else {
            std::cout << "Task not found.\n";
        }
        pauseAndClear();
    }
};

int main() {
    try {
        TaskManager manager;
        manager.run();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
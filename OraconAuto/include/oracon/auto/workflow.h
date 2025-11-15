#ifndef ORACON_AUTO_WORKFLOW_H
#define ORACON_AUTO_WORKFLOW_H

#include "oracon/auto/agent.h"
#include <functional>
#include <vector>

namespace oracon {
namespace auto_ns {

// Task status
enum class TaskStatus {
    Pending,
    Running,
    Completed,
    Failed,
    Skipped
};

// Task result
struct TaskResult {
    String output;
    bool succeeded;  // Renamed from 'success' to avoid conflict with static method
    String error;
    std::any data;  // Additional data from task execution

    TaskResult() : succeeded(false) {}

    bool isSuccess() const { return succeeded; }

    static TaskResult success(const String& output) {
        TaskResult result;
        result.succeeded = true;
        result.output = output;
        return result;
    }

    static TaskResult failure(const String& error) {
        TaskResult result;
        result.succeeded = false;
        result.error = error;
        return result;
    }
};

// Single task in a workflow
class Task {
public:
    using TaskFunction = std::function<TaskResult()>;

    Task(const String& name, const String& description, TaskFunction func)
        : m_name(name)
        , m_description(description)
        , m_function(func)
        , m_status(TaskStatus::Pending)
    {}

    // Execute the task
    TaskResult execute() {
        m_status = TaskStatus::Running;

        try {
            m_result = m_function();
            m_status = m_result.succeeded ? TaskStatus::Completed : TaskStatus::Failed;
        } catch (const std::exception& e) {
            m_result = TaskResult::failure(String("Exception: ") + e.what());
            m_status = TaskStatus::Failed;
        }

        return m_result;
    }

    // Getters
    const String& getName() const { return m_name; }
    const String& getDescription() const { return m_description; }
    TaskStatus getStatus() const { return m_status; }
    const TaskResult& getResult() const { return m_result; }

private:
    String m_name;
    String m_description;
    TaskFunction m_function;
    TaskStatus m_status;
    TaskResult m_result;
};

// Workflow - a sequence of tasks
class Workflow {
public:
    explicit Workflow(const String& name) : m_name(name) {}

    // Add a task
    Workflow& addTask(const String& name, const String& description, Task::TaskFunction func) {
        m_tasks.push_back(std::make_unique<Task>(name, description, func));
        return *this;
    }

    // Add an agent task
    Workflow& addAgentTask(const String& name, const String& query, std::shared_ptr<Agent> agent) {
        return addTask(name, "Agent task: " + query, [agent, query]() -> TaskResult {
            AgentResult result = agent->execute(query);
            if (result.isSuccess()) {
                return TaskResult::success(result.finalResponse);
            } else {
                return TaskResult::failure(result.error);
            }
        });
    }

    // Execute the workflow
    bool execute() {
        bool allSucceeded = true;

        for (auto& task : m_tasks) {
            TaskResult result = task->execute();

            if (!result.succeeded) {
                allSucceeded = false;
                // Optionally stop on first failure
                // break;
            }
        }

        return allSucceeded;
    }

    // Get workflow status
    String getStatusReport() const {
        String report = "Workflow: " + m_name + "\n";
        report += "==================\n\n";

        for (const auto& task : m_tasks) {
            String statusStr;
            switch (task->getStatus()) {
                case TaskStatus::Pending: statusStr = "PENDING"; break;
                case TaskStatus::Running: statusStr = "RUNNING"; break;
                case TaskStatus::Completed: statusStr = "COMPLETED"; break;
                case TaskStatus::Failed: statusStr = "FAILED"; break;
                case TaskStatus::Skipped: statusStr = "SKIPPED"; break;
            }

            report += "[" + statusStr + "] " + task->getName() + "\n";
            report += "  " + task->getDescription() + "\n";

            if (task->getStatus() == TaskStatus::Completed) {
                report += "  Output: " + task->getResult().output + "\n";
            } else if (task->getStatus() == TaskStatus::Failed) {
                report += "  Error: " + task->getResult().error + "\n";
            }

            report += "\n";
        }

        return report;
    }

    const String& getName() const { return m_name; }
    const std::vector<std::unique_ptr<Task>>& getTasks() const { return m_tasks; }

private:
    String m_name;
    std::vector<std::unique_ptr<Task>> m_tasks;
};

// Workflow builder for easier construction
class WorkflowBuilder {
public:
    WorkflowBuilder(const String& name) : m_workflow(std::make_unique<Workflow>(name)) {}

    WorkflowBuilder& step(const String& name, const String& description, Task::TaskFunction func) {
        m_workflow->addTask(name, description, func);
        return *this;
    }

    WorkflowBuilder& agentStep(const String& name, const String& query, std::shared_ptr<Agent> agent) {
        m_workflow->addAgentTask(name, query, agent);
        return *this;
    }

    std::unique_ptr<Workflow> build() {
        return std::move(m_workflow);
    }

private:
    std::unique_ptr<Workflow> m_workflow;
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_WORKFLOW_H

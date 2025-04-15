#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <cmath>

using namespace emscripten;

struct Workout {
    std::string exercise;
    int duration; // in minutes
    std::string intensity; // "Low", "Moderate", "High"
};

struct User {
    std::string name;
    int age;
    double height; // in cm
    double weight; // in kg
    int dailyCalorieIntake;
    std::string fitnessGoal; // "WeightLoss", "MuscleGain", "MaintainWeight"
};

class FitnessTracker {
private:
    User user;
    std::vector<Workout> workouts;
    bool userSet;

public:
    FitnessTracker() : userSet(false) {}

    void setUser(std::string name, int age, double height, double weight, int dailyCalorieIntake, std::string fitnessGoal) {
        user = {name, age, height, weight, dailyCalorieIntake, fitnessGoal};
        userSet = true;
    }

    void addWorkout(std::string exercise, int duration, std::string intensity) {
        workouts.push_back({exercise, duration, intensity});
    }

    double getBMR() {
        if (!userSet) return 0.0;
        // Mifflin-St Jeor Equation: BMR = 10*weight + 6.25*height - 5*age + s (s = +5 for males, -161 for females)
        // Assuming gender-neutral average (s = -78)
        return 10.0 * user.weight + 6.25 * user.height - 5.0 * user.age - 78.0;
    }

    double getDailyCalorieBurn() {
        if (!userSet) return 0.0;
        // Assume moderately active (multiply BMR by 1.55)
        return getBMR() * 1.55;
    }

    double getTargetCalories() {
        if (!userSet) return 0.0;
        double dailyBurn = getDailyCalorieBurn();
        if (user.fitnessGoal == "WeightLoss") {
            // 500 kcal deficit for ~1 lb/week loss
            return dailyBurn - 500.0;
        } else if (user.fitnessGoal == "MuscleGain") {
            // 500 kcal surplus for muscle gain
            return dailyBurn + 500.0;
        } else { // MaintainWeight
            return dailyBurn;
        }
    }

    double getWorkoutCalories() {
        double totalCalories = 0.0;
        for (const auto& workout : workouts) {
            double met = 3.0; // Default MET (Metabolic Equivalent of Task)
            if (workout.intensity == "Low") met = 3.0; // e.g., light walking
            else if (workout.intensity == "Moderate") met = 5.0; // e.g., jogging
            else if (workout.intensity == "High") met = 8.0; // e.g., running
            // Calorie burn = MET * weight (kg) * duration (hours)
            totalCalories += met * user.weight * (workout.duration / 60.0);
        }
        return totalCalories;
    }

    std::string getFeedback() {
        if (!userSet || workouts.empty()) return "Please set user details and add workouts.";
        double target = getTargetCalories();
        double actual = user.dailyCalorieIntake + getWorkoutCalories();
        double dailyBurn = getDailyCalorieBurn();
        double netCalories = actual - dailyBurn; // Net calories after accounting for burn

        if (user.fitnessGoal == "WeightLoss") {
            if (netCalories <= target) {
                return "Great job! You're on track for weight loss.";
            } else {
                return "You're slightly over your calorie target. Try reducing intake or increasing workout intensity.";
            }
        } else if (user.fitnessGoal == "MuscleGain") {
            if (netCalories >= target) {
                return "Awesome! You're hitting your calorie surplus for muscle gain.";
            } else {
                return "You're under your calorie target. Increase your intake or add more workouts.";
            }
        } else { // MaintainWeight
            if (std::abs(netCalories - target) < 100) {
                return "Perfect! You're maintaining your weight.";
            } else if (netCalories > target) {
                return "Slightly over your maintenance calories. Consider lighter meals.";
            } else {
                return "Slightly under your maintenance calories. Add a small snack.";
            }
        }
    }

    std::string getSummary() {
        if (!userSet) return "No user details set.";
        std::string summary = "User: " + user.name + "\n";
        summary += "Daily Calorie Intake: " + std::to_string(user.dailyCalorieIntake) + " kcal\n";
        summary += "Estimated Daily Burn: " + std::to_string(static_cast<int>(getDailyCalorieBurn())) + " kcal\n";
        summary += "Target Calories: " + std::to_string(static_cast<int>(getTargetCalories())) + " kcal\n";
        summary += "Workouts:\n";
        for (const auto& workout : workouts) {
            summary += workout.exercise + ": " + std::to_string(workout.duration) + " min (" + workout.intensity + ")\n";
        }
        summary += "Workout Calories: " + std::to_string(static_cast<int>(getWorkoutCalories())) + " kcal\n";
        summary += "Feedback: " + getFeedback();
        return summary;
    }

    void clearWorkouts() {
        workouts.clear();
    }
};

EMSCRIPTEN_BINDINGS(fitness_tracker) {
    class_<FitnessTracker>("FitnessTracker")
        .constructor<>()
        .function("setUser", &FitnessTracker::setUser)
        .function("addWorkout", &FitnessTracker::addWorkout)
        .function("getBMR", &FitnessTracker::getBMR)
        .function("getDailyCalorieBurn", &FitnessTracker::getDailyCalorieBurn)
        .function("getTargetCalories", &FitnessTracker::getTargetCalories)
        .function("getWorkoutCalories", &FitnessTracker::getWorkoutCalories)
        .function("getFeedback", &FitnessTracker::getFeedback)
        .function("getSummary", &FitnessTracker::getSummary)
        .function("clearWorkouts", &FitnessTracker::clearWorkouts);
}
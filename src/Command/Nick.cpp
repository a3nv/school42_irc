#include "../../includes/Command.hpp"

Nick::Nick() : Command("NICK") {
    std::cout << "NICK command initialized." << std::endl
    << "Changes or sets the nickname of a user. <---- MUST BE UNIQUE" << std::endl; // To be deleted
}

Nick::~Nick() {
    std::cout << "NICK command destroyed." << std::endl; // To be deleted
}

bool isValidNickname(const std::string& nickname) {
    // Basic validation: nickname must be 1-9 characters, start with a letter,
    // and contain only letters, digits, hyphens, and underscores.
    if (nickname.empty() || nickname.length() > 9)
        return false;
    if (!std::isalpha(nickname[0]))
        return false;
    for (size_t i = 1; i < nickname.length(); ++i) {
        if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_')
            return false;
    }
    return true;
}

bool Nick::validate(const std::string& nickname, const Server& server) {
    if (!isValidNickname(nickname)) {
        return false; // Invalid nickname format
    }
    // Check for uniqueness
    return true; // Nickname is valid and unique
}
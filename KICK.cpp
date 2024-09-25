#include "Server.hpp"
#include <sstream>
#include <algorithm> // For std::remove

void FindK(std::string cmd, std::string tofind, std::string &str) {
	size_t i = 0;
	for (; i < cmd.size(); i++) {
		if (cmd[i] != ' ') {
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind) break;
			else tmp.clear();
		}
	}
	if (i < cmd.size()) str = cmd.substr(i);
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

std::string SplitCmdK(std::string &cmd) {
	std::stringstream ss(cmd);
	std::string reason;
	


	// The remaining part is the reason
	if (std::getline(ss, reason)) {
		size_t pos = reason.find(':');
		if (pos != std::string::npos) {
			reason = reason.substr(pos + 1); // Extract everything after ":"
		}
	}

	// Trim leading and trailing spaces
	reason.erase(0, reason.find_first_not_of(' '));
	reason.erase(reason.find_last_not_of(' ') + 1);

	return reason;
}

std::string Server::SplitCmdKick(std::string cmd, std::string &channel, std::vector<std::string> &users, int fd) {
	std::stringstream ss(cmd);
	std::string str, reason;
	if (!(ss >> str)) return std::string("");
	if (!(ss >> str)) return std::string("");
	if (ss.peek() != '#' && str[0] == '#')
		channel = str;
	else {
		ss >> channel;
	}
	if (!(ss >> str)) return std::string("");
	std::cout << "LOOKING FOR USERS : " << std::endl; 
	std::cout << str << std::endl; 
	if (str[0] == '#')
	{
		channel = str;
		ss >> str;
	}
	std::cout << str << std::endl;
	if (str[0] == ':')
	{
		std::cout << "FOUND : " << std::endl; 
		str = str.substr(1);
		std::cout << str << std::endl; 
	}
	if (str.find(',') != std::string::npos){ // Split the user string by commas to get individual users
		std::stringstream ssn(str);
		std::string user;
		std::cout << "FOUND ," << std::endl; 
		while (std::getline(ssn, user, ',')) {
		users.push_back(user);
		}
	}
	else {
	users.push_back(str);
	}
	for (size_t i = 0; i < users.size(); i++)
	{
		std::cout << users[i] << std::endl;
	}
	
	std::cout << str << std::endl;
	
	if (std::getline(ss, reason))
	{
		size_t pos = reason.find_first_not_of(' ');
		if (pos != std::string::npos)
			reason = reason.substr(pos);
	}
	

	// Validate channel
	if (channel.empty() || channel[0] != '#') {
		senderror(403, GetClient(fd)->GetNickName(), channel, GetClient(fd)->Getfd(), " :No such channel\r\n");
		return std::string("");
	}
	else
		channel = channel.substr(1);

	return reason;
}

void Server::KICK(std::string cmd, int fd) {
	std::cout << cmd << std::endl;
	std::string channel;
	std::vector<std::string> users;
	std::string reason = SplitCmdKick(cmd, channel, users, fd);
	std::cout << channel << std::endl;
	for (size_t i = 0; i < users.size(); i++)
	{
		std::cout << users[i] << std::endl;
	}
	
	if (users.empty() || channel.empty()) {
		senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->Getfd(), " :Not enough parameters\r\n");
		return;
	}

	Channel *ch = GetChannel(channel);
	if (!ch) {
		senderror(403, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :No such channel\r\n");
		return;
	}

	if (!ch->get_client(fd) && !ch->get_admin(fd)) {
		senderror(442, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :You're not on that channel\r\n");
		return;
	}

	if (!ch->get_admin(fd)) {
		senderror(482, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :You're not channel operator\r\n");
		return;
	}

	
	for (std::vector<std::string>::iterator it = users.begin(); it != users.end(); ++it) {
		std::string &user = *it; // Dereference iterator to get user

		Client *clientToKick = ch->GetClientInChannel(user);
		if (clientToKick) {
		if (clientToKick->Getfd() == fd){
			sendResponse("for kick youself, use please command PASS", fd);
			continue;
		}
			std::stringstream ss;
			ss << ":" << GetClient(fd)->GetNickName() << "!~@" << "localhost" << " KICK #" << channel << " " << user;
			if (!reason.empty())
				ss << " :" << reason << "\r\n";
			else
				ss << "\r\n";
			ch->sendTo_all(ss.str());

			if (ch->get_admin(clientToKick->Getfd())) {
				ch->remove_admin(clientToKick->Getfd());
			} else {
				ch->remove_client(clientToKick->Getfd());
			}
		} else {
			senderror(441, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :They aren't on that channel\r\n");
		}
	}
}

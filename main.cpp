#include <iostream>
#include <thread>
#include <string>
#include <boost/asio.hpp>
#include <conio.h>
#include <windows.h>

std::string history = "";
std::string userInput = "";
int userNameLen = 0;
std::string userName = "";

int PORT = 2003;

std::mutex mutex;

void historyUpdate(std::string& str) {
	history += str;
}

void userInputUpdate(char ch) {
	if (ch == '\b') {
		if (userInput.size() > userNameLen + 2) {
			userInput.resize(userInput.size() - 1);
		}
	}
	else {
		userInput += ch;
	}
}

void displayUpdate() {
	mutex.lock();

	system("cls");
	std::cout << history;
	//std::cout << std::endl;
	std::cout << userInput;

	mutex.unlock();
}

void sendMessage() {
	using boost::asio::ip::udp;
	try {
		// Создаем I/O контекст
		boost::asio::io_context io_context;

		// Создаем UDP сокет
		udp::socket socket(io_context);
		socket.open(udp::v4());

		// Устанавливаем параметры широковещательной рассылки
		socket.set_option(boost::asio::socket_base::broadcast(true));

		// Указываем широковещательный адрес (например, 255.255.255.255) и порт
		udp::endpoint broadcast_endpoint(boost::asio::ip::address::from_string("255.255.255.255"), PORT);

		// Отправляем сообщение
		socket.send_to(boost::asio::buffer(userInput), broadcast_endpoint);
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void client(std::string userName) {

	userInput = userName;
	userInput += ": ";
	displayUpdate();

	while (1) {
		char ch = _getch();
		if (ch == '\r') {
			ch = '\n';
		}

		userInputUpdate(ch);

		if (ch == '\n' && userInput.size() == userNameLen + 3) {
			userInput = userName;
			userInput += ": ";
		}
		else if (ch == '\n') {
			sendMessage();

			userInput = userName;
			userInput += ": ";
		}
		displayUpdate();
	}
}

void server() {
	using boost::asio::ip::udp;
	try {
		// Создаем I/O контекст
		boost::asio::io_context io_context;

		// Создаем UDP сокет
		udp::socket socket(io_context, udp::endpoint(udp::v4(), PORT));

		while (true) {
			// Буфер для приема сообщения
			char buffer[2048];
			udp::endpoint sender_endpoint;

			// Принимаем сообщение
			size_t length = socket.receive_from(boost::asio::buffer(buffer), sender_endpoint);

			// Преобразуем полученное сообщение в строку
			std::string message(buffer, length);
			historyUpdate(message);
			displayUpdate();
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

int main(int args, char* argv[]) {

	if (args == 2) {
		userName = argv[1];
	}
	else if (args == 3) {
		PORT = std::stoi(argv[2]);
	}
	else {
		std::cout << "Enter your user Name: ";
		std::getline(std::cin, userName);
	}
	userNameLen = userName.size();

	std::thread cli(client, userName);
	std::thread ser(server);

	cli.join();
	ser.join();

	return 0;
}

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Funktio lainausmerkkien poistoon merkkijonosta
std::string removeQuotes(const std::string& str) {
  if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
}

// Funktio JSON tiedoston jäsentämiseen
std::vector<std::map<std::string, std::string>>
parseJson(const std::string &content) {
  std::vector<std::map<std::string, std::string>> data;
  size_t pos = 0;

  while ((pos = content.find("{", pos)) != std::string::npos) {
    size_t end = content.find("}", pos);
    if (end == std::string::npos)
      break;

    std::string objStr = content.substr(pos + 1, end - pos - 1);
    std::map<std::string, std::string> record;
    size_t keyPos = 0;

    while ((keyPos = objStr.find("\"", keyPos)) != std::string::npos) {
      size_t keyStart = keyPos + 1;
      size_t keyEnd = objStr.find("\"", keyStart);
      if (keyEnd == std::string::npos) break;
      
      std::string key = objStr.substr(keyStart, keyEnd - keyStart);

      size_t valueStart = objStr.find(":", keyEnd) + 1;
      while (valueStart < objStr.length() && 
             (objStr[valueStart] == ' ' || objStr[valueStart] == '\t')) {
        valueStart++;
      }

      size_t valueEnd;
      std::string value;
      
      if (objStr[valueStart] == '"') {
        // Lainausmerkkien jäsennys
        valueStart++;
        valueEnd = objStr.find("\"", valueStart);
        value = objStr.substr(valueStart, valueEnd - valueStart);
        valueEnd++;
      } else {
        // Numeroiden ja muiden arvojen jäsennys
        valueEnd = objStr.find(",", valueStart);
        if (valueEnd == std::string::npos) {
          valueEnd = objStr.length();
        }
        value = objStr.substr(valueStart, valueEnd - valueStart);
        // Välilyöntien poisto
        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
      }

      record[key] = value;
      keyPos = valueEnd;
    }

    data.push_back(record);
    pos = end + 1;
  }

  return data;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./5GmetricsJSONtoExcel <input.json> <output.csv>\n";
    return 1;
  }

  // JSON tiedoston avaaminen
  std::ifstream inFile(argv[1]);
  if (!inFile.is_open()) {
    std::cerr << "Error: Cannot open input file.\n";
    return 1;
  }

  // Tallenna JSON tiedoston sisältö muistiin
  std::stringstream buffer;
  buffer << inFile.rdbuf();
  std::string content = buffer.str();
  inFile.close();

  // Kutsu JSON sisällön jäsennykseen
  std::vector<std::map<std::string, std::string>> data = parseJson(content);

  if (data.empty()) {
    std::cerr << "Error: No data found in input JSON.\n";
    return 1;
  }

  // Avaa CSV tiedosto kirjoittamista varten
  std::ofstream outFile(argv[2]);
  if (!outFile.is_open()) {
    std::cerr << "Error: Cannot open output file.\n";
    return 1;
  }

  // Kolumnien järjestyksen määritys
  std::vector<std::string> headers = {
    "dl_throughput_mbps",
    "ul_throughput_mbps",
    "connected_users",
    "handover_success_rate",
    "drop_call_rate",
    "latency_ms",
    "packet_loss_rate",
    "cpu_utilization_percent",
    "memory_utilization_percent",
    "timestamp"
  };

  // Otsikon lisäys CSV tiedostoon
  for (size_t i = 0; i < headers.size(); ++i) {
    outFile << headers[i];
    if (i != headers.size() - 1)
      outFile << ",";
  }
  outFile << "\n";

  // JSON sisällön kirjoitus CSV tiedostoon
  for (const auto &record : data) {
    for (size_t i = 0; i < headers.size(); ++i) {
      const auto &key = headers[i];
      auto it = record.find(key);
      if (it != record.end()) {
        outFile << it->second;
      }
      if (i != headers.size() - 1)
        outFile << ",";
    }
    outFile << "\n";
  }

  outFile.close();
  std::cout << "CSV file written to " << argv[2] << "\n";
  return 0;
}
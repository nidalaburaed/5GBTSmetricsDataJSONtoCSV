#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>
#include <cmath>

std::string removeQuotes(const std::string& str) {
  if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
}

std::vector<std::map<std::string, std::string>> parseJson(const std::string &content) {
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
        valueStart++;
        valueEnd = objStr.find("\"", valueStart);
        value = objStr.substr(valueStart, valueEnd - valueStart);
        valueEnd++;
      } else {
        valueEnd = objStr.find(",", valueStart);
        if (valueEnd == std::string::npos) {
          valueEnd = objStr.length();
        }
        value = objStr.substr(valueStart, valueEnd - valueStart);
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

double toDouble(const std::string &val) {
  try {
    return std::stod(val);
  } catch (...) {
    return 0.0;
  }
}

void streamKPIs(const std::vector<std::map<std::string, std::string>> &data) {
  std::cout << "\n===== Point-in-Time KPIs =====\n";
  std::cout << std::fixed << std::setprecision(2);

  for (size_t i = 0; i < data.size(); ++i) {
    const auto &record = data[i];

    double dl = toDouble(record.at("dl_throughput_mbps"));
    double ul = toDouble(record.at("ul_throughput_mbps"));
    double users = toDouble(record.at("connected_users"));
    double cpu = toDouble(record.at("cpu_utilization_percent"));
    double mem = toDouble(record.at("memory_utilization_percent"));
    double dropCall = toDouble(record.at("drop_call_rate"));
    double latency = toDouble(record.at("latency_ms"));

    double ul_dl_ratio = (dl != 0.0) ? ul / dl : 0.0;
    double per_user_dl = (users != 0.0) ? dl / users : 0.0;
    double per_user_ul = (users != 0.0) ? ul / users : 0.0;
    double throughput_eff_cpu = (cpu != 0.0) ? (dl + ul) / cpu : 0.0;
    double throughput_eff_mem = (mem != 0.0) ? (dl + ul) / mem : 0.0;
    double call_reliability = 100.0 - dropCall;
    double latency_throughput_ratio = ((dl + ul) != 0.0) ? latency / (dl + ul) : 0.0;
    double resource_load_index = (cpu + mem) / 2.0;
    double user_load_efficiency = ((cpu + mem) != 0.0) ? users / (cpu + mem) : 0.0;

    std::cout << "Sample #" << i + 1 << " [Timestamp: " << record.at("timestamp") << "]\n";
    std::cout << " - UL/DL Ratio: " << ul_dl_ratio << "\n";
    std::cout << " - Per User DL (Mbps): " << per_user_dl << "\n";
    std::cout << " - Per User UL (Mbps): " << per_user_ul << "\n";
    std::cout << " - Throughput Efficiency per CPU: " << throughput_eff_cpu << "\n";
    std::cout << " - Throughput Efficiency per Memory: " << throughput_eff_mem << "\n";
    std::cout << " - Call Reliability Index (%): " << call_reliability << "\n";
    std::cout << " - Latency/Throughput Ratio: " << latency_throughput_ratio << "\n";
    std::cout << " - Resource Load Index: " << resource_load_index << "\n";
    std::cout << " - User Load Efficiency: " << user_load_efficiency << "\n\n";
  }

  std::cout << "=========================================\n";
}

void printKPISummary(const std::vector<std::map<std::string, std::string>> &data) {
  double sumDL = 0, sumUL = 0, sumLatency = 0, minLatency = std::numeric_limits<double>::max(), maxLatency = 0;
  double sumPacketLoss = 0, sumDropCall = 0, sumCPU = 0, sumMem = 0, sumUsers = 0;
  double sumHandoverSuccess = 0;
  int count = 0;

  std::vector<std::pair<std::string, std::pair<double, double>>> peakHourRecords;
  std::vector<std::pair<std::string, std::pair<double, double>>> nonPeakHourRecords;

  std::vector<double> dropRates;

  for (const auto &record : data) {
    double dl = toDouble(record.at("dl_throughput_mbps"));
    double ul = toDouble(record.at("ul_throughput_mbps"));
    double latency = toDouble(record.at("latency_ms"));
    double packetLoss = toDouble(record.at("packet_loss_rate"));
    double dropCall = toDouble(record.at("drop_call_rate"));
    double cpu = toDouble(record.at("cpu_utilization_percent"));
    double mem = toDouble(record.at("memory_utilization_percent"));
    double users = toDouble(record.at("connected_users"));
    double hoSuccess = toDouble(record.at("handover_success_rate"));

    std::string timestamp = record.at("timestamp");
    int hour = std::stoi(timestamp.substr(11, 2));

    if (hour >= 18 && hour < 22)
      peakHourRecords.emplace_back(timestamp, std::make_pair(dl, ul));
    else
      nonPeakHourRecords.emplace_back(timestamp, std::make_pair(dl, ul));

    sumDL += dl;
    sumUL += ul;
    sumLatency += latency;
    sumPacketLoss += packetLoss;
    sumDropCall += dropCall;
    sumCPU += cpu;
    sumMem += mem;
    sumUsers += users;
    sumHandoverSuccess += hoSuccess;

    minLatency = std::min(minLatency, latency);
    maxLatency = std::max(maxLatency, latency);
    ++count;

    dropRates.push_back(dropCall);
  }

  double avgDropCall = sumDropCall / count;
  double avgUsers = sumUsers / count;
  double avgHO = sumHandoverSuccess / count;

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "\n===== 5G Metrics Summary =====\n";
  std::cout << "Average DL Throughput (Mbps): " << sumDL / count << "\n";
  std::cout << "Average UL Throughput (Mbps): " << sumUL / count << "\n";
  std::cout << "Min Latency (ms): " << minLatency << "\n";
  std::cout << "Max Latency (ms): " << maxLatency << "\n";
  std::cout << "Average Latency (ms): " << sumLatency / count << "\n";
  std::cout << "Average Packet Loss Rate (%): " << sumPacketLoss / count << "\n";
  std::cout << "Average Drop Call Rate (%): " << avgDropCall << "\n";
  std::cout << "Average Handover Success Rate (%): " << avgHO << "\n";
  std::cout << "Average CPU Utilization (%): " << sumCPU / count << "\n";
  std::cout << "Average Memory Utilization (%): " << sumMem / count << "\n";
  std::cout << "Average Connected Users: " << avgUsers << "\n";

  std::cout << "\n===== Anomaly: Throughput Drop During Peak Hours =====\n";
  if (nonPeakHourRecords.empty()) {
    std::cout << "Insufficient non-peak data.\n";
  } else {
    double avgDL = 0, avgUL = 0;
    for (const auto& rec : nonPeakHourRecords) {
      avgDL += rec.second.first;
      avgUL += rec.second.second;
    }
    avgDL /= nonPeakHourRecords.size();
    avgUL /= nonPeakHourRecords.size();

    bool anomaly = false;
    for (const auto& rec : peakHourRecords) {
      double dl = rec.second.first;
      double ul = rec.second.second;
      if (dl < 0.7 * avgDL || ul < 0.7 * avgUL) {
        anomaly = true;
        std::cout << "Anomaly at " << rec.first << ": DL=" << dl << ", UL=" << ul << "\n";
      }
    }

    if (!anomaly)
      std::cout << "No throughput drop anomalies detected.\n";
  }

  std::cout << "\n===== Anomaly: High Latency with Low Resource Load =====\n";
  bool latencyLoadAnomaly = false;
  for (const auto& record : data) {
    double latency = toDouble(record.at("latency_ms"));
    double cpu = toDouble(record.at("cpu_utilization_percent"));
    double mem = toDouble(record.at("memory_utilization_percent"));
    double avgLoad = (cpu + mem) / 2.0;

    if (latency > 100.0 && avgLoad < 30.0) {
      latencyLoadAnomaly = true;
      std::cout << "Anomaly at " << record.at("timestamp")
                << ": Latency = " << latency
                << " ms, CPU = " << cpu
                << "%, Memory = " << mem
                << "% (Avg. Load = " << avgLoad << "%)\n";
    }
  }

  if (!latencyLoadAnomaly)
    std::cout << "No high-latency/low-load anomalies detected.\n";

  std::cout << "\n===== Anomaly: Call Drop Rate Spike With Stable Users and HO Success =====\n";
  bool spikeDetected = false;

  for (size_t i = 1; i < data.size(); ++i) {
    double prevDrop = toDouble(data[i - 1].at("drop_call_rate"));
    double currDrop = toDouble(data[i].at("drop_call_rate"));
    double currUsers = toDouble(data[i].at("connected_users"));
    double currHO = toDouble(data[i].at("handover_success_rate"));

    if ((currDrop > 2.0 * prevDrop || currDrop > avgDropCall * 1.5) &&
        std::abs(currUsers - avgUsers) / avgUsers < 0.1 &&
        std::abs(currHO - avgHO) / avgHO < 0.1) {
      spikeDetected = true;
      std::cout << "Spike at " << data[i].at("timestamp")
                << ": Drop Call Rate = " << currDrop
                << "%, Connected Users = " << currUsers
                << ", Handover Success = " << currHO << "%\n";
    }
  }

  if (!spikeDetected)
    std::cout << "No call drop spikes with stable conditions detected.\n";

    std::cout << "\n===== Anomaly: High CPU/Memory Spike with No Traffic Growth =====\n";
    bool highCpuMemSpikeAnomaly = false;

    for (size_t i = 1; i < data.size(); ++i) {
        double prevDL = toDouble(data[i - 1].at("dl_throughput_mbps"));
        double currDL = toDouble(data[i].at("dl_throughput_mbps"));
        double prevUL = toDouble(data[i - 1].at("ul_throughput_mbps"));
        double currUL = toDouble(data[i].at("ul_throughput_mbps"));
        double prevCPU = toDouble(data[i - 1].at("cpu_utilization_percent"));
        double currCPU = toDouble(data[i].at("cpu_utilization_percent"));
        double prevMem = toDouble(data[i - 1].at("memory_utilization_percent"));
        double currMem = toDouble(data[i].at("memory_utilization_percent"));

        bool cpuSpike = (currCPU > 1.2 * prevCPU);  // 20% spike
        bool memSpike = (currMem > 1.2 * prevMem);  // 20% spike

        bool trafficGrowth = (currDL > 1.1 * prevDL || currUL > 1.1 * prevUL);  // 10% growth

        if ((cpuSpike || memSpike) && !trafficGrowth) {
            highCpuMemSpikeAnomaly = true;
            std::cout << "Anomaly at " << data[i].at("timestamp")
                    << ": CPU = " << currCPU
                    << "%, Memory = " << currMem
                    << "%, DL = " << currDL
                    << " Mbps, UL = " << currUL
                    << " Mbps (No significant traffic growth)\n";
        }
    }

    if (!highCpuMemSpikeAnomaly)
        std::cout << "No high CPU/Memory spikes with no traffic growth detected.\n";

  std::cout << "=======================================================================\n";
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./5GmetricsJSONtoExcel <input.json> <output.csv>\n";
    return 1;
  }

  std::ifstream inFile(argv[1]);
  if (!inFile.is_open()) {
    std::cerr << "Error: Cannot open input file.\n";
    return 1;
  }

  std::stringstream buffer;
  buffer << inFile.rdbuf();
  std::string content = buffer.str();
  inFile.close();

  std::vector<std::map<std::string, std::string>> data = parseJson(content);
  if (data.empty()) {
    std::cerr << "Error: No data found in input JSON.\n";
    return 1;
  }

  streamKPIs(data);

  std::ofstream outFile(argv[2]);
  if (!outFile.is_open()) {
    std::cerr << "Error: Cannot open output file.\n";
    return 1;
  }

  std::vector<std::string> headers = {
    "dl_throughput_mbps", "ul_throughput_mbps", "connected_users",
    "handover_success_rate", "drop_call_rate", "latency_ms",
    "packet_loss_rate", "cpu_utilization_percent",
    "memory_utilization_percent", "timestamp"
  };

  for (size_t i = 0; i < headers.size(); ++i) {
    outFile << headers[i];
    if (i != headers.size() - 1) outFile << ",";
  }
  outFile << "\n";

  for (const auto &record : data) {
    for (size_t i = 0; i < headers.size(); ++i) {
      const auto &key = headers[i];
      auto it = record.find(key);
      if (it != record.end()) {
        outFile << it->second;
      }
      if (i != headers.size() - 1) outFile << ",";
    }
    outFile << "\n";
  }

  outFile.close();

  printKPISummary(data);

  std::cout << "CSV file written to " << argv[2] << "\n";
  return 0;
}
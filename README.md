# Description

C++-based script that converts 5G NR (vRAN) call-related metrics data file (JSON) content into Excel (CSV) format. The script also has comprehensive monitoring features to support the network operator in their daily work while doing the conversion. The script calculates the in-point of time KPIs (UL/DL Ratio, Per User DL (Mbps), Per User UL (Mbps), Throughput Efficiency per CPU, Throughput Efficiency per Memory, Call Reliability Index, Latency/Throughput Ratio, Resource Load Index, User Load Efficiency) from every sample of the JSON data at a specific point in time. After the inpoint KPIs calculation, it prints the summary KPIs (Average DL Throughput (Mbps), Average UL Throughput (Mbps), Min Latency (ms), Max Latency (ms), Average Latency (ms), Average Packet Loss Rate (%), Average Drop Call Rate (%), Average Handover Success Rate (%), Average CPU Utilization (%), Average Memory Utilization (%), Average Connected Users) over the full dataset. Within the same function (printkpisummary) the script also executes 4 anomaly detection KPIs (Throughput Drop During Peak Hours, High Latency with Low Resource Load, Call Drop Rate Spike with Stable Users and HO Success, High CPU/Memory Spike with No Traffic Growth). Finally, the script writes the JSON-content into a csv-file.

The script is doing this because it is designed to support the network operator with his/hers very specialized needs. This serving of the operator is done by spotting very known anomalies to the network operator using very alarming "buzzwords" in order to simulate instant recovery actions reponse for common problems widely known among network operators. These are most common and usually the first place to investigate:

1. Radio Network Level (Typical Causes)

a. Coverage Holes / Interference
	•	Caused by physical obstructions, weather, or misaligned antennas.
	•	Often localized and can be seen with spike patterns in certain cells.

b. Handover Failures
	•	If X2/N2 handover fails due to bad target cell config or signaling issues.
	•	Sudden drop in mobility success + spike in drops.

c. Timing Issues
	•	TA (Timing Advance) misconfigurations leading to RLF (Radio Link Failure).
	•	May relate to incorrect synchronization with GPS/1588 clocks.

d. Excessive Load / Resource Congestion
	•	PRB (Physical Resource Block) exhaustion.
	•	High UE density with insufficient scheduling capacity or license limits.

e. HARQ/NACK Issues
	•	HARQ processes failing due to bad link adaptation or interference.
	•	Low DL BLER + drop spike can be a hint.

⸻

2. Core Network / Interface Issues

Less common, but worth checking during area-wide spikes.

a. Session Management Fails (SMF/UPF)
	•	If PDU session fails mid-call, the drop appears to radio as an RLF.
	•	Look at NG-C signaling failures (N2 interface).

b. AMF Paging or Registration Rejection
	•	If mobility is blocked due to identity mismatch or TAC issues.

⸻

3. Server/Software-Level Causes (on the gNB side)

a. Memory Cache Exhaustion or Leaks
	•	TCP/IP stack buffer issues, EPC anchor process leak, or bad LRU policies.
	•	Often seen with slowly building issues until sudden tipping point.

b. Thread Starvation or Core Overload
	•	gNB processing threads over-allocated or stuck (like RRC or MAC threads).
	•	High context switch rate, high core temperature, or stuck mutexes.

c. gNB Application or FAPI Driver Fault
	•	Abnormal behavior from PHY/MAC interface (via FAPI/IFx).
	•	Related to real-time scheduling or DPDK-related drops.

⸻

4. Transport / Backhaul Issues
	•	High latency, jitter, or packet loss between DU and CU.
	•	Common in split option 7.x or 2 when fronthaul is not guaranteed low-latency.

⸻

5. Software Upgrade/Config Anomalies
	•	Bad config pushed via EMS (wrong PCI/TA/NRARFCN).
	•	Incomplete restart after upgrade, e.g., partial component active.

![image](https://github.com/user-attachments/assets/29d2afd0-4b65-4b01-b396-b4912668190c)

# Instructions to compile and run the code

Compile the code using the following command:

```bash
g++ -o 5GmetricsJSONtoExcel 5GmetricsJSONtoExcel.cpp .
```

Run the code using the following command:

```bash
./5GmetricsJSONtoExcel <input.json> <output.csv>
```

# Notes

Example JSON files are provided in the repository

## Inquiries

In case of inquiries, feel free to send a message via email to nidalaburaed94@gmail.com

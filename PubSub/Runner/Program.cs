
using System.Diagnostics;

namespace Run {
    internal class Program {
        static List<Process> processes = new List<Process>();

        static void Main(string[] args) {
            ManyClients();

            Console.WriteLine("\nPress any key to exit.\n");
            Console.ReadKey();
            KillAll();
        }

        static void OneClient() {
            Process client = CreateProcess("Client", "0", "0", "760", "840");
            Process broker = CreateProcess("Broker", "760", "0", "760", "840");

            processes.Add(broker);
            processes.Add(client);

            broker.Start();
            client.Start();
        }

        static void ManyClients() {
            Process client1 = CreateProcess("Client", "0", "0", "350", "205");
            Process client2 = CreateProcess("Client", "0", "205", "350", "205");
            Process client3 = CreateProcess("Client", "0", "410", "350", "205");
            Process client4 = CreateProcess("Client", "0", "615", "350", "205");

            Process client5 = CreateProcess("Client", "350", "0", "350", "205");
            Process client6 = CreateProcess("Client", "350", "205", "350", "205");
            Process client7 = CreateProcess("Client", "350", "410", "350", "205");
            Process client8 = CreateProcess("Client", "350", "615", "350", "205");

            Process client9 = CreateProcess("Client", "700", "0", "350", "205");
            Process client10 = CreateProcess("Client", "700", "205", "350", "205");
            Process client11 = CreateProcess("Client", "700", "410", "350", "205");
            Process client12 = CreateProcess("Client", "700", "615", "350", "205");

            Process broker = CreateProcess("Broker", "1050", "0", "490", "840");

            processes.Add(broker);
            processes.Add(client1);
            processes.Add(client2);
            processes.Add(client3);
            processes.Add(client4);
            processes.Add(client5);
            processes.Add(client6);
            processes.Add(client7);
            processes.Add(client8);
            processes.Add(client9);
            processes.Add(client10);
            processes.Add(client11);
            processes.Add(client12);

            broker.Start();
            client1.Start();
            client2.Start();
            client3.Start();
            client4.Start();
            client5.Start();
            client6.Start();
            client7.Start();
            client8.Start();
            client9.Start();
            client10.Start();
            client11.Start();
            client12.Start();
        }

        static void KillAll() {
            foreach (Process process in processes) {
                process.Kill();
            }
        }

        static Process CreateProcess(string program, string x, string y, string width, string height) {
            return new Process {
                StartInfo = new ProcessStartInfo {
                    FileName = "C:\\Users\\User\\Desktop\\fakultet\\cetvrta godina\\IKP\\projekat\\PubSub\\x64\\Debug\\" + program + "IKP.exe",
                    UseShellExecute = true,
                    CreateNoWindow = false,
                    ArgumentList = { x, y, width, height }
                }
            };
        }
    }
}
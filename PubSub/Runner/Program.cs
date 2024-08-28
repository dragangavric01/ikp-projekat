
using System.Diagnostics;

namespace Run {
    internal class Program {
        static List<Process> processes = new List<Process>();

        static void Main(string[] args) {
            Start();

            Console.WriteLine("\nPress any key to exit.\n");
            Console.ReadKey();
            KillAll();
        }

        static void Start() {
            Process client1 = CreateProcess("ClientIKP.exe", "0", "0", "525", "275", "0");
            Process client2 = CreateProcess("ClientIKP.exe", "0", "275", "525", "275", "0");
            Process client3 = CreateProcess("ClientIKP.exe", "0", "550", "525", "275", "0");

            //Process client4 = CreateProcess("ClientIKP.exe", "525", "0", "525", "275", "0");
            //Process client5 = CreateProcess("ClientIKP.exe", "525", "275", "525", "275", "0");
            Process client6 = CreateProcess("ClientIKP.exe", "525", "550", "525", "275", "0");

            Process sm = CreateProcess("StatsManager.exe", "525", "0", "525", "550", "0");

            Process broker = CreateProcess("BrokerIKP.exe", "1050", "0", "490", "840", "0");

            processes.Add(sm);
            processes.Add(broker);
            processes.Add(client1);
            processes.Add(client2);
            processes.Add(client3);
            processes.Add(client6);

            sm.Start();
            broker.Start();
            client1.Start();
            client2.Start();
            client3.Start();
            client6.Start();
        }

        static void KillAll() {
            foreach (Process process in processes) {
                process.Kill();
            }
        }

        static Process CreateProcess(string program, string x, string y, string width, string height, string clientNumber) {
            return new Process {
                StartInfo = new ProcessStartInfo {
                    FileName = "C:\\Users\\User\\Desktop\\fakultet\\cetvrta godina\\prvi semestar\\IKP\\projekat\\PubSub\\x64\\Debug\\" + program,
                    UseShellExecute = true,
                    CreateNoWindow = false,
                    ArgumentList = { x, y, width, height, clientNumber }
                }
            };
        }
    }
}
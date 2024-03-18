using System;
using System.Text;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Management.Automation.Runspaces;

namespace powerpick
{
    internal class Program
    {
        static void Main(string[] args)
        {
            string cmd = string.Join(" ", args);

            PS ps = new PS();
            string output = ps.exe(cmd);
            ps.close();

            Console.Write(output);
        }
    }

    /* class taken from https://github.com/p3nt4/PowerShdll/blob/master/exe/Common.cs */
    public class PS
    {
        Runspace runspace;

        public PS()
        {
            this.runspace = RunspaceFactory.CreateRunspace();
            this.runspace.Open();
        }

        public string exe(string cmd)
        {
            try {
                Pipeline pipeline = runspace.CreatePipeline();
                pipeline.Commands.AddScript(cmd);
                pipeline.Commands.Add("Out-String");

                Collection<PSObject> results = pipeline.Invoke();
                StringBuilder stringBuilder = new StringBuilder();

                foreach (PSObject obj in results)
                    foreach (string line in obj.ToString().Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.None))
                        stringBuilder.AppendLine(line.TrimEnd());

                return stringBuilder.ToString();
            } catch (Exception e) {
                string errorText = e.Message + "\n";
                return (errorText);
            }
        }

        public void close()
        {
            this.runspace.Close();
        }
    }
}

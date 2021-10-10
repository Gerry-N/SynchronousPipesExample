using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SyncPipeClient
{
	public class PipeClient
	{
		private static Thread worker;
		public const string pipe_name = "server_data";
		PipeClient(bool auto_start = true)
		{
			if (auto_start)
			{
				StartClient();
				worker.Join();
			}
		}
		public static void StartClient()
		{
			worker = new Thread(PipeClient.ThreadProc);
			worker.Start();
		}
		private static void ThreadProc()
		{
			NamedPipeClientStream pipe = new NamedPipeClientStream(pipe_name);
			pipe.Connect();
			pipe.ReadMode = PipeTransmissionMode.Message;
			TextReader textReader = new StreamReader(pipe);
			TextWriter textWriter = new StreamWriter(pipe);

			uint cmd_counter = 0;

			try
			{
				string line = textReader.ReadLine();
				Trace.WriteLine(line);
			}
			catch (Exception ex)
			{
				Trace.WriteLine(ex.ToString());
				StartClient();
				return;
			}
			for (; ;)
			{
				try
				{
					if(cmd_counter == 100)
					{
						textWriter.WriteLine("stop");
					}
					else
					{
						textWriter.WriteLine("Command #" + cmd_counter + ".");
					}
					textWriter.Flush();
					cmd_counter++;
				}
				catch(Exception ex)
				{
					Trace.WriteLine(ex.ToString());
					break;
				}
				try
				{
					string line = textReader.ReadLine();
					Trace.WriteLine(line);
					if (line == "stop")
					{
						return;
					}
				}
				catch (Exception ex)
				{
					Trace.WriteLine(ex.ToString());
					break;
				}
			}
			Thread.Sleep(250);
			StartClient();
		}
	}
}

using EnvDTE;
using EnvDTE80;
using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.GameDev
{
    public enum BuildConfiguration
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor,
    }

    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progID = "VisualStudio.DTE.18.0";
        private static readonly object _lock = new object();
        private static ManualResetEventSlim _resetEvent = new ManualResetEventSlim(false);
        private static readonly string[] _buildConfigurationNames = new string[]
        {
            "Debug",
            "DebugEditor",
            "Release",
            "ReleaseEditor"
        };

        public static string GetConfigurationName(BuildConfiguration config) => _buildConfigurationNames[(int)config];

        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);
        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable prot);

        private static void CallOnSTATThread(Action action)
        {
            Debug.Assert(action != null);
            var thread = new System.Threading.Thread(() =>
            {
                MessageFilter.Register();
                try
                {
                    action();
                }
                catch(Exception ex)
                {
                    Logger.Log(MessageType.Warning, ex.Message);
                }
                finally 
                { 
                    MessageFilter.Revoke();
                }
            });

            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            thread.Join();
        }

        private static void OpenVisualStudio_Internal(string solutionPath)
        {
            IRunningObjectTable runningObjectTable = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindingContext = null;
            try
            {
                if (_vsInstance == null)
                {
                    var hResult = GetRunningObjectTable(0, out runningObjectTable);
                    if (hResult < 0 || runningObjectTable == null)
                        throw new COMException($"GetRunningObjectTable() returned HRESULT: {hResult:X8}");

                    runningObjectTable.EnumRunning(out monikerTable);
                    monikerTable.Reset();

                    hResult = CreateBindCtx(0, out bindingContext);
                    if (hResult < 0 || bindingContext == null)
                        throw new COMException($"CreateBindCtx() returned HRESULT: {hResult:X8}");

                    IMoniker[] currentMoniker = new IMoniker[1];
                    while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                    {
                        string name = string.Empty;
                        currentMoniker[0]?.GetDisplayName(bindingContext, null, out name);
                        if (name.Contains(_progID))
                        {
                            hResult = runningObjectTable.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null)
                                throw new COMException($"Running object table's GetObject() returned  HResult: {hResult:X8}");
                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            var solutionName = string.Empty;
                            CallOnSTATThread(() =>
                            {
                                solutionName = dte.Solution.FullName;
                            });
                            if (solutionName == solutionPath)
                            {
                                _vsInstance = dte;
                                break;
                            }
                        }
                    }

                    if (_vsInstance == null)
                    {
                        Type visualStudioType = Type.GetTypeFromProgID(_progID, true);
                        _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, "Failed to open the Visual Studio");
            }
            finally
            {
                if (monikerTable != null)
                    Marshal.ReleaseComObject(monikerTable);
                if (runningObjectTable != null)
                    Marshal.ReleaseComObject(runningObjectTable);
                if (bindingContext != null)
                    Marshal.ReleaseComObject(bindingContext);
            }
        }

        public static void OpenVisualStudio(string solutionPath)
        {
            lock (_lock)
            {
                OpenVisualStudio_Internal(solutionPath);
            }
        }

        private static void CloseVisualStudio_Internal()
        {
            CallOnSTATThread(() =>
            {
                if (_vsInstance?.Solution.IsOpen == true)
                {
                    _vsInstance.ExecuteCommand("File.SaveAll");
                    _vsInstance.Solution.Close(true);
                }
                _vsInstance.Quit();
                _vsInstance = null;
            });
        }

        public static void CloseVisualStudio()
        {
            lock (_lock)
            {
                CloseVisualStudio_Internal();
            }
        }

        private static bool AddFilesToSolution_Internal(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio_Internal(solution);
            try
            {
                if (_vsInstance != null)
                {
                    CallOnSTATThread(() =>
                    {
                        if (!_vsInstance.Solution.IsOpen)
                            _vsInstance.Solution.Open(solution);
                        else
                            _vsInstance.ExecuteCommand("File.SaveAll");

                        foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
                        {
                            if (project.UniqueName.Contains(projectName))
                            {
                                foreach (var file in files)
                                {
                                    project.ProjectItems.AddFromFile(file);
                                }
                            }
                        }

                        var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                        if (!string.IsNullOrEmpty(cpp))
                        {
                            _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                        }
                        _vsInstance.MainWindow.Activate();
                        _vsInstance.MainWindow.Visible = true;
                    });
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debug.WriteLine("Failed to add files to the Visual Studio solution");
                return false;
            }
            return true;
        }

        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            lock (_lock)
            {
                return AddFilesToSolution_Internal(solution, projectName, files);
            }
        }

        private static void OnBuildSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool success)
        {
            if (BuildDone)
                return;
            if (success)
                Logger.Log(MessageType.Info, $"Building {projectConfig} configuration succeeded.");
            else
                Logger.Log(MessageType.Info, $"Building {projectConfig} configuration failed.");

            BuildDone = true;
            BuildSucceeded = success;
            _resetEvent.Set();
        }

        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            if (BuildDone)
                return;
            Logger.Log(MessageType.Info, $"Building {project}, {projectConfig}, {platform}, {solutionConfig}.");
        }

        private static bool IsDebugging_Internal()
        {
            bool result = false;
            CallOnSTATThread(() =>
            {
                result = _vsInstance != null && (_vsInstance.Debugger.CurrentProgram != null ||
                    _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
            });
            return result;
        }

        public static bool IsDebugging()
        {
            lock (_lock)
            {
                return IsDebugging_Internal();
            }
        }

        private static void BuildSolution_Internal(GameProject.Backend.Project project, BuildConfiguration buildConfig, bool showWindow = true)
        {
            if (IsDebugging_Internal())
            {
                Logger.Log(MessageType.Error, "Visual Studio is currently running a process.");
                return;
            }

            OpenVisualStudio_Internal(project.Solution);
            BuildDone = BuildSucceeded = false;
            CallOnSTATThread(() =>
            {
                if (!_vsInstance.Solution.IsOpen)
                    _vsInstance.Solution.Open(project.Solution);

                _vsInstance.MainWindow.Visible = showWindow;
                _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;
            });

            var configName = GetConfigurationName(buildConfig);

            try
            {
                foreach (var pdbFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{configName}"), ".pdb"))
                {
                    File.Delete(pdbFile);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            CallOnSTATThread(() =>
            {
                _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                _vsInstance.ExecuteCommand("Build.BuildSolution");
                _resetEvent.Wait();
                _resetEvent.Reset();
            });
        }

        public static void BuildSolution(GameProject.Backend.Project project, BuildConfiguration buildConfig, bool showWindow = true)
        {
            lock (_lock)
            {
                BuildSolution_Internal(project, buildConfig, showWindow);
            }
        }

        private static void Run_Internal(GameProject.Backend.Project project, BuildConfiguration buildConfig, bool debug)
        {
            CallOnSTATThread(() =>
            {
                if (_vsInstance != null && !IsDebugging_Internal() && BuildSucceeded)
                {
                    _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");

                }
            });
        }

        public static void Run(GameProject.Backend.Project project, BuildConfiguration buildConfig, bool debug)
        {
            Run_Internal(project, buildConfig, debug);
        }

        private static void Stop_Internal()
        {
            CallOnSTATThread(() =>
            {
                if (_vsInstance != null && IsDebugging_Internal())
                {
                    _vsInstance.ExecuteCommand("Debug.StopDebugging");
                }
            });
        }

        public static void Stop()
        {
            lock (_lock)
            {
                Stop_Internal();
            }
        }
    }

    //Class containing the IOleMessageFilter thread error-handling function
    public class MessageFilter : IMessageFilter
    {
        private const int SERVERCALL_ISHANDLED = 0;
        private const int PENDINGMSG_WAITDEFPROCESS = 2;
        private const int SERVERCALL_RETRYLATER = 2;

        [DllImport("Ole32.dll")]
        private static extern int CoRegisterMessageFilter(IMessageFilter newFilter, out IMessageFilter oldFilter);

        public static void Register()
        {
            IMessageFilter newFilter = new MessageFilter();
            int hr = CoRegisterMessageFilter(newFilter, out var oldFilter);
            Debug.Assert(hr >= 0, "Registering COM IMessageFilter failed.");
        }

        public static void Revoke()
        {
            int hr = CoRegisterMessageFilter(null, out var oldFilter);
            Debug.Assert(hr >= 0, "Unregistering COM IMessageFilter failed.");
        }

        int IMessageFilter.HandleInComingCall(int dwCallType, nint hTaskCaller, int dwTickCount, nint lpInterfaceInfo)
        {
            //Returns the flag SERVERCALL_ISHANDLED.
            return SERVERCALL_ISHANDLED;
        }

        int IMessageFilter.RetryRejectedCall(nint hTaskCallee, int dwTickCount, int dwRejectType)
        {
            //Thread call was refused, try again.
            if(dwRejectType == SERVERCALL_RETRYLATER)
            {
                //Retry thread call at once, if return value >=0 & <100.
                Debug.WriteLine("COM server busy. Retrying call to EnvDTE interface.");
                return 500;
            }
            //Too busy. Cancel call.
            return -1;
        }

        int IMessageFilter.MessagePending(nint hTaskCallee, int dwTickCount, int dwPendingType)
        {
            return PENDINGMSG_WAITDEFPROCESS;
        }
    }

    [ComImport(), Guid("00000016-0000-0000-C000-000000000046"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IMessageFilter
    {
        [PreserveSig]
        int HandleInComingCall(int dwCallType, IntPtr hTaskCaller, int dwTickCount, IntPtr lpInterfaceInfo);

        [PreserveSig]
        int RetryRejectedCall(IntPtr hTaskCallee, int dwTickCount, int dwRejectType);

        [PreserveSig]
        int MessagePending(IntPtr hTaskCallee, int dwTickCount, int dwPendingType);
    }
    
}

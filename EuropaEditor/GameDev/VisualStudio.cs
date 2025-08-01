﻿using EnvDTE;
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
    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progID = "VisualStudio.DTE.17.0";

        public static bool BuildSucceeded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);
        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable prot);

        public static void OpenVisualStudio(string solutionPath)
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
                            var solutionName = dte.Solution.FullName;
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

        public static void CloseVisualStudio()
        {
            if (_vsInstance?.Solution.IsOpen == true)
            {
                _vsInstance.ExecuteCommand("File.SaveAll");
                _vsInstance.Solution.Close();
            }
            _vsInstance?.Quit();
        }

        internal static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solution);
            try
            {
                if (_vsInstance != null)
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
        }

        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            Logger.Log(MessageType.Info, $"Building {project}, {projectConfig}, {platform}, {solutionConfig}.");
        }

        public static bool IsDebugging()
        {
            bool result = false;
            bool tryAgain = true;
            for (int i = 0; i < 3 && tryAgain; ++i)
            {
                try
                {
                    result = _vsInstance != null && (_vsInstance.Debugger.CurrentProgram != null || _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
                    tryAgain = false;
                }
                catch (Exception ex)
                {
                    Debug.Write(ex.Message);
                    System.Threading.Thread.Sleep(10);
                }
            }
            return result;
        }

        internal static void BuildSolution(GameProject.Backend.Project project, string configName, bool showWindow = true)
        {
            if (IsDebugging())
            {
                Logger.Log(MessageType.Error, "Visual Studio is currently running a process.");
                return;
            }

            OpenVisualStudio(project.Solution);
            BuildDone = BuildSucceeded = false;
            for (int i = 0; i < 3 && !BuildDone; ++i)
            {
                try
                {
                    if (!_vsInstance.Solution.IsOpen)
                        _vsInstance.Solution.Open(project.Solution);
                    _vsInstance.MainWindow.Visible = showWindow;

                    _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;

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
                    _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                    _vsInstance.ExecuteCommand("Build.BuildSolution");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Debug.WriteLine($"Attempt: failed to build {project.Name}");
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        public static void Run(GameProject.Backend.Project project, string configName, bool debug)
        {
            if (_vsInstance != null && !IsDebugging() && BuildDone && BuildSucceeded)
            {
                _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");

            }
        }

        public static void Stop()
        {
            if (_vsInstance != null && IsDebugging())
            {
                _vsInstance.ExecuteCommand("Debug.StopDebugging");
            }
        }
    }
}

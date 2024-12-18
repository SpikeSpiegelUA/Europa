using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Interop;
using System.Windows.Shapes;

namespace EuropaEditor.Utilities
{
    enum MessageType
    {
        Info = 0x01,
        Warning = 0x02,
        Error = 0x02,

    }

    class LogMessage
    {
        public DateTime Time { get; }
        public MessageType TypeOfMessage { get; }
        public string Message { get; }
        public string File { get; }
        public string Caller { get; }
        public int Line { get; }
        public string MetaData => $"{File}: {Caller} ({Line})";

        public LogMessage(MessageType messageType, string message, string file, string caller, int line)
        {
            Time = DateTime.Now;
            TypeOfMessage = messageType;
            Message = message;
            File = System.IO.Path.GetFileName(file);
            Caller = caller;
            Line = line;
        }
    }

    static class Logger
    {
        private static int _messageFilter = (int)(MessageType.Info | MessageType.Warning | MessageType.Error);
        private readonly static ObservableCollection<LogMessage> _logMessages = new ObservableCollection<LogMessage>();
        public static ReadOnlyObservableCollection<LogMessage> LogMessages 
        { get; } = new ReadOnlyObservableCollection<LogMessage>(_logMessages);

        public static CollectionViewSource FilteredMessages { get; } = new CollectionViewSource() { Source = LogMessages };

        public static async void Log(MessageType messageType, string msg, [CallerFilePath]string file = "", 
            [CallerMemberName]string caller = "", [CallerLineNumber] int line = 0)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                _logMessages.Add(new LogMessage(messageType, msg, file, caller, line));
            }));
        }

        public static async void Clear()
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                _logMessages.Clear();
            }));
        }

        public static void SetMessageFilter(int mask)
        {
            _messageFilter = mask;
            FilteredMessages.View.Refresh();
        }

        static Logger()
        {
            FilteredMessages.Filter += (s, e) =>
            {
                var type = (int)(e.Item as LogMessage).TypeOfMessage;
                e.Accepted = (_messageFilter & type) != 0;
            };
        }
    }
}

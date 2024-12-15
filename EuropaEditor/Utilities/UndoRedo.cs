using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Utilities
{
    public interface IUndoRedo
    {
        string Name { get; }
        void Undo();
        void Redo();
    }

    public class UndoRedoAction : IUndoRedo
    {
        private Action _undoAction;
        private Action _redoAction;

        public string Name { get; }
        public void Undo() => _undoAction?.Invoke();
        public void Redo() => _redoAction?.Invoke();

        public UndoRedoAction(string name)
        {
            Name = name;
        }

        public UndoRedoAction(Action undoAction, Action redoAction, string name) : this(name)
        {
            Debug.Assert(undoAction != null && redoAction != null);
            _undoAction = undoAction;
            _redoAction = redoAction;
        }

        public UndoRedoAction(string propertyName, object instance, object undoValue, object redoValue, string undoRedoActionName) 
            : this( () => instance.GetType().GetProperty(propertyName).SetValue(instance, undoValue), 
                    () => instance.GetType().GetProperty(propertyName).SetValue(instance, redoValue),
                    undoRedoActionName)
        {}
    }

    public class UndoRedo
    {
        private ObservableCollection<IUndoRedo> _redoList = new ObservableCollection<IUndoRedo>();
        private ObservableCollection<IUndoRedo> _undoList = new ObservableCollection<IUndoRedo>();

        public ReadOnlyObservableCollection<IUndoRedo> RedoList { get; }
        public ReadOnlyObservableCollection<IUndoRedo> UndoList { get; }

    private bool _enableAdd = true;

    public void Reset()
        {
            _undoList.Clear();
            _redoList.Clear();
        }

        public void AddUndo(IUndoRedo cmd)
        {
            if (_enableAdd)
            {
                _undoList.Add(cmd);
                _redoList.Clear();
            }
        }

        public void Undo()
        {
            if (_undoList.Any())
            {
                var cmd = _undoList.Last();
                _undoList.RemoveAt(_undoList.Count - 1);
                _enableAdd = false;
                cmd.Undo();
                _enableAdd = true;
                _redoList.Insert(0, cmd);
            }
        }

        public void Redo()
        {
            if (_redoList.Any())
            {
                var cmd = _redoList.First();
                _redoList.RemoveAt(0);
                _enableAdd = false;
                cmd.Redo();
                _enableAdd = true;
                _undoList.Add(cmd);
            }
        }


        public UndoRedo()
        {
            RedoList = new ReadOnlyObservableCollection<IUndoRedo>(_redoList);
            UndoList = new ReadOnlyObservableCollection<IUndoRedo>(_undoList);
        }
    }
}

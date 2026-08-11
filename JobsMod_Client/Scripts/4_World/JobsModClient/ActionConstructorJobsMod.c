// ActionConstructorJobsMod.c
//
// Every custom DayZ ActionBase must be constructed by ActionConstructor before
// PlayerBase.AddAction() can put it into an input-action map. Without this,
// GetActionManager().GetAction(ActionTalkToNpc) returns null and the interaction
// can never become available.

modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);

        if (actions.Find(ActionSortTrash) == -1)
            actions.Insert(ActionSortTrash);

        if (actions.Find(ActionTalkToNpc) == -1)
            actions.Insert(ActionTalkToNpc);

        if (actions.Find(ActionUseLocker) == -1)
            actions.Insert(ActionUseLocker);

        JobsLog.Info("WORLD/ACTIONS: JobsMod actions зарегистрированы: ActionSortTrash, ActionTalkToNpc.");
    }
}

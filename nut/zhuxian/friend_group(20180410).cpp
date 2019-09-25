#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <set>

//cost a lot:
//and chat
//todo: add logs, protocols

typedef std::string Octets;
struct Protocol
{
    /* data */
};
void LOG_TRACE(const char *format,...)
{
}
struct Timer
{
    static time_t GetTime(){return 0;}
};


enum
{
    DEFAULT_CAPACITY_PER_CHAT = 35,
    DEFAULT_OWN_LIMIT_PER_ROLE = 3,
    DEFAULT_JOIN_LIMIT_PER_ROLE = 30,

    MAX_CAPACITY_PER_CHAT = 75,
    MAX_OWN_LIMIT_PER_ROLE = 10,
    MAX_JOIN_LIMIT_PER_ROLE = 35,

    DEFAULT_SESSION_TIMEOUT = 10,
    UPDATE_ROLE_SESSION_TIMEOUT = 600,

    //MAX_SIZE_OF_CHAT_INTRODUCTION = 30,
    //MAX_APPLICANT_PER_CHAT = 50,
};

struct ChatRoleInfo
{
    Octets name;
    int zoneid;
    int roleid;
    short level;
    char cultivation;
    char occupation;
};

struct FC_Role
{
    //Octets name;
    int zoneid;
    int roleid;
    //short level;
    //char cultivation;
    //char occupation;
    short own_chats_addon;//move to origial
    short join_chats_addon;
    time_t last_active_time;
    std::map<int,bool> own_chats;//key is fc_chatid,value is ignore_msg
    std::map<int,bool> join_chats;//key is fc_chatid,value is ignore_msg
    FC_Role(Octets name_t,int zoneid_t,int roleid_t,short level_t,char cultivation_t,char occupation_t)
        :/*name(name_t),*/zoneid(zoneid_t),roleid(roleid_t),//level(level_t),cultivation(cultivation_t),
        //occupation(occupation_t),
        own_chats_addon(0),join_chats_addon(0)
    {}
    FC_Role(ChatRoleInfo &info):/*name(info.name),*/zoneid(info.zoneid),roleid(info.roleid),//level(info.level),cultivation(info.cultivation),
    //occupation(info.occupation),
    own_chats_addon(0),join_chats_addon(0)
    {}
    FC_Role(){}//to delete
    size_t get_own_limit()
    {
        return own_chats_addon + DEFAULT_OWN_LIMIT_PER_ROLE;
    }
    size_t get_join_limit()
    {
        return join_chats_addon + DEFAULT_JOIN_LIMIT_PER_ROLE;
    }
};
typedef std::map<int,bool>::iterator role_chats_iterator;

struct FC_ChatRole
{
    Octets name;
    bool ignore_msg;
    int zoneid;
    int roleid;
    short level;
    char cultivation;
    char occupation;
    bool online;//not save,not very exact
    FC_ChatRole(const FC_ChatRole &apply):name(apply.name),ignore_msg(false),
    zoneid(apply.zoneid),roleid(apply.roleid),level(apply.level),
    cultivation(apply.cultivation),occupation(apply.occupation),online(true)
    {}
    FC_ChatRole():name("12345677654321"){}//to delete
};
struct FC_Chat
{
    Octets chat_name;
    Octets introduction;
    int owner;
    short capacity_addon;
    char confirmation;//验证方式:拒绝全部>无需验证>允许全部/好友/帮会/家族
    bool can_search;
    bool is_delete;
    time_t last_active_time;

    std::map<int,FC_ChatRole> members;//key is fc_roleid

    std::set<std::pair<int,int> > inviter;//<inviter,chatid>,not save
    std::map<int,FC_ChatRole> applicant;//key is fc_roleid,not save

    FC_Chat(Octets name_t,int owner_t):chat_name(name_t),owner(owner_t),capacity_addon(0),confirmation(0),can_search(false),is_delete(false)
    {}

    void update_member_info(int fc_roleid, ChatRoleInfo &info)
    {
        std::map<int,FC_ChatRole>::iterator it = members.find(fc_roleid);
        if(it == members.end())
        {
            //logerr
            return;
        }
        it->second.name = info.name;
        it->second.zoneid = info.zoneid;
        it->second.roleid = info.roleid;
        it->second.level = info.level;
        it->second.cultivation = info.cultivation;
        it->second.occupation = info.occupation;
        it->second.online = true;
    }
    void update_member_online(int fc_roleid, bool online)
    {
        std::map<int,FC_ChatRole>::iterator it = members.find(fc_roleid);
        if(it == members.end())
        {
            //logerr
            return;
        }
        it->second.online = online;
    }
    size_t get_capacity_limit()
    {
        return capacity_addon + DEFAULT_CAPACITY_PER_CHAT;
    }
};
typedef std::map<int,FC_ChatRole>::iterator chat_members_iterator;


class FC_Server
{
public://to delete
    std::map<int,FC_Role> _roles;
    std::map<int,FC_Chat> _chats;
    typedef std::map<int,FC_Role>::iterator role_iterator;
    typedef std::map<int,FC_Chat>::iterator chat_iterator;
    std::set<int> _tosave_chats;//key is fc_chatid
    std::map<int,ChatRoleInfo> _toupdate_roles;//key is fc_roleid
    std::multimap<Octets,int> _name_map;//value is fc_chatid
    std::multimap<int,int> _offline_roles;//<fc_roleid,fc_chatid>
    int _next_roleid;
    int _next_chatid;
    bool _save_nextid;
    bool _ready;

    class delay_session
    {
        std::map<int,time_t> _sessions;
        time_t _timeout;
    public:
        enum {
            SESSION_OK = 0,
            SESSION_NEW = 1,
            SESSION_TIMEOUT = 2,
        };
        delay_session(time_t timeout = DEFAULT_SESSION_TIMEOUT):_timeout(timeout){}
        int can_do(int id, time_t now)
        {
            std::map<int,time_t>::iterator it = _sessions.find(id);
            if (it == _sessions.end())
            {
                _sessions.insert(std::pair<int,time_t>(id,now));
                return SESSION_NEW;
            }
            if (it->second + _timeout > now)
            {
                it->second = now;
                return SESSION_TIMEOUT;
            }
            return SESSION_OK;
        }
        void erase(int id)
        {
            _sessions.erase(id);
        }
    };
    delay_session _loading_chats_session;
    delay_session _updating_role_session;

    void update_member_online(int fc_chatid, int fc_roleid, bool online)
    {
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        it->second.update_member_online(fc_roleid, online);
    }
    enum {
        UPDATE_NOT_FOUND = 0,
        UPDATE_NOT_CHANGED = 1,
        UPDATE_CHANGED = 2,
    };
    //用户信息没有变化时,set_online为true表示要设置其为在线
    //用户信息有变化时,set_online值为什么都要设置其为在线
    int update_roleinfo(int fc_roleid, ChatRoleInfo &info, bool set_online)
    {
        role_iterator it = _roles.find(fc_roleid);
        if(it != _roles.end())
        {
            FC_Role &role = it->second;
            if(/*role.name == info.name && */role.zoneid == info.zoneid &&
                    role.roleid == info.roleid) //&& role.level == info.level &&
                    //role.cultivation == info.cultivation && role.occupation == info.occupation)
            {
                if(set_online)
                {
                    role_chats_iterator it2 = role.own_chats.begin();
                    for(; it2!=role.own_chats.end(); ++it2)
                        update_member_online(it2->first, fc_roleid, true);
                    it2 = role.join_chats.begin();
                    for(; it2!=role.join_chats.end(); ++it2)
                        update_member_online(it2->first, fc_roleid, true);
                }
                return UPDATE_NOT_CHANGED;
            }
            else
            {
                //if changed
                //role.name = info.name;
                role.zoneid = info.zoneid;
                role.roleid = info.roleid;
                //role.level = info.level;
                //role.cultivation = info.cultivation;
                //role.occupation = info.occupation;

                role_chats_iterator it2 = role.own_chats.begin();
                for(; it2!=role.own_chats.end(); ++it2)
                {
                    chat_iterator it3 = _chats.find(it2->first);
                    if(it3 == _chats.end())
                    {
                        //logerr
                        continue;
                    }
                    it3->second.update_member_info(fc_roleid,info);
                    _tosave_chats.insert(it2->first);
                }
                it2 = role.join_chats.begin();
                for(; it2!=role.join_chats.end(); ++it2)
                {
                    chat_iterator it3 = _chats.find(it2->first);
                    if(it3 == _chats.end())
                    {
                        //logerr
                        continue;
                    }
                    it3->second.update_member_info(fc_roleid,info);
                    _tosave_chats.insert(it2->first);
                }
                return UPDATE_CHANGED;
            }
        }
        return UPDATE_NOT_FOUND;
    }
    void save_chats_step(std::vector<int> &chats)
    {
        for(std::vector<int>::iterator it=chats.begin(); it!=chats.end(); ++it)
            DoChatMemberSave(*it);
    }

public:
    FC_Server():_next_roleid(1024),_next_chatid(1024),_save_nextid(false),_ready(false),_updating_role_session(UPDATE_ROLE_SESSION_TIMEOUT)
    {
    }

    void OnDBConnect()
    {
        //GameDBClient::GetInstance()->SendProtocol(Rpc::Call());
        //when timeout resend this rpc
    }
    void SetNextId(int next_roleid, int next_chatid)
    {
        LOG_TRACE("set next id,ready=%d,roleid=%d,chatid=%d",_ready,next_roleid,next_chatid);
        if(!_ready)
        {
            if(next_roleid) _next_roleid = next_roleid;
            if(next_chatid) _next_chatid = next_chatid;
            _ready = true;
        }
    }

    int AllocRoleid()
    {
        _save_nextid = true;
        return _next_roleid++;
    }
    int AllocChatid()
    {
        _save_nextid = true;
        return _next_chatid++;
    }

    void DoNextidSave()
    {
        if(!_save_nextid) return;
        //serialize and send to db
    }
    void DoRoleSave(int fc_roleid, FC_Role &role)
    {
        //serialize role
        //send role-save rpc to db
    }
    void DoRoleSave(int fc_roleid)
    {
        role_iterator it = _roles.find(fc_roleid);
        if (it == _roles.end()) return;
        DoRoleSave(fc_roleid,it->second);
    }
    void DoRoleSave(std::vector<int> &fc_roleid)
    {
        //serialize and send
    }
    void DoChatSave(int fc_chatid, FC_Chat &chat)
    {
        //serialize chat
        //send chat-save rpc to db
    }
    void DoChatSave(int fc_chatid)
    {
        chat_iterator it = _chats.find(fc_chatid);
        if (it == _chats.end()) return;
        DoChatSave(fc_chatid,it->second);
    }
    void DoChatMemberSave(int fc_chatid, std::map<int,FC_ChatRole> &members)
    {
        //serialize member
        //send member-save rpc to db
        _tosave_chats.erase(fc_chatid);
    }
    void DoChatMemberSave(int fc_chatid)
    {
        chat_iterator it = _chats.find(fc_chatid);
        if (it == _chats.end()) return;
        DoChatMemberSave(fc_chatid,it->second.members);
    }

    void OnPlayerOn()
    {
        int fc_roleid;
        ChatRoleInfo info;

        int ret = _updating_role_session.can_do(fc_roleid, Timer::GetTime());
        if(ret == delay_session::SESSION_OK)
        {
            std::multimap<int,int>::iterator it = _offline_roles.find(fc_roleid);
            while(it!=_offline_roles.end())
            {
                if(it->first != fc_roleid) break;
                update_member_online(it->second,fc_roleid,true);
                _offline_roles.erase(it++);
            }
            return;
        }
        if(ret == delay_session::SESSION_TIMEOUT) _offline_roles.erase(fc_roleid);
        ret = update_roleinfo(fc_roleid, info, ret == delay_session::SESSION_NEW);
        if(ret == UPDATE_CHANGED)
        {
            DoRoleSave(fc_roleid);
        }
        else if(ret == UPDATE_NOT_FOUND)
        {
            _toupdate_roles.insert(std::make_pair(fc_roleid,info));
            //send to db to load role
        }
    }
    void OnLoadRole(int fc_roleid, FC_Role role)
    {
        bool save_flag = false;
        std::vector<int> loading_chats;
        std::vector<int> toload_chats;
        role_chats_iterator it = role.own_chats.begin();
        while(it != role.own_chats.end())
        {
            chat_iterator it2 = _chats.find(it->first);
            if(it2 == _chats.end())
            {
                if(_loading_chats_session.can_do(it->first,Timer::GetTime()))
                    toload_chats.push_back(it->first);
                else
                    loading_chats.push_back(it->first);
                ++it;
            }
            else if(it2->second.is_delete)
            {
                role.own_chats.erase(it++);
                save_flag = true;
            }
            else
            {
                ++it;
            }
        }
        it = role.join_chats.begin();
        while(it != role.join_chats.end())
        {
            chat_iterator it2 = _chats.find(it->first);
            if(it2 == _chats.end())
            {
                if(_loading_chats_session.can_do(it->first,Timer::GetTime()))
                    toload_chats.push_back(it->first);
                else
                    loading_chats.push_back(it->first);
                ++it;
            }
            else if(it2->second.is_delete)
            {
                role.join_chats.erase(it++);
                save_flag = true;
            }
            else
            {
                ++it;
            }
        }

        std::pair<role_iterator,bool> ret = _roles.insert(std::make_pair(fc_roleid,role));
        if(!ret.second)
        {
            //logerr
        }
        if(save_flag) DoRoleSave(fc_roleid,ret.first->second);

        //send to db to load toload_chats
    }
    void OnLoadChats()
    {
        int fc_roleid;
        std::vector<int> loading_chats;
        std::map<int,FC_Chat> chats;

        //按道理,在toload_chats返回的时候,loading_chats都已加载完毕了才对

        bool role_exist = true;
        bool save_role = false;
        role_iterator it = _roles.find(fc_roleid);
        if(it == _roles.end())
        {
            role_exist = false;
            //logerr
        }

        for(chat_iterator it2 = chats.begin(); it2 != chats.end(); ++it2)
        {
            _loading_chats_session.erase(it2->first);
            if(role_exist && it2->second.is_delete)
            {
                it->second.own_chats.erase(it2->first) || it->second.join_chats.erase(it2->first);
                save_role = true;
            }
            _chats.insert(*it2);
            _name_map.insert(std::make_pair(it2->second.chat_name,it2->first));
        }

        if(!role_exist) return;

        FC_Role &role = it->second;
        for(std::vector<int>::iterator it3 = loading_chats.begin(); it3 != loading_chats.end(); ++it3)
        {
            chat_iterator it4 = _chats.find(*it3);
            if(it4 == _chats.end())
            {
                //logerr
                continue;
            }
            if(it4->second.is_delete)
            {
                role.own_chats.erase(*it3);
                role.join_chats.erase(*it3);
                save_role = true;
            }
        }

        std::map<int,ChatRoleInfo>::iterator it5 = _toupdate_roles.find(fc_roleid);
        if(it5 == _toupdate_roles.end())
        {
            if(save_role) DoRoleSave(fc_roleid,role);
        }
        else
        {
            if(UPDATE_CHANGED == update_roleinfo(fc_roleid, it5->second, true) || save_role) DoRoleSave(fc_roleid);
            _toupdate_roles.erase(it5);
        }
    }

    //when cannot find player in origimal delivery
    void SetRoleOffline(int fc_roleid, int fc_chatid)
    {
        update_member_online(fc_chatid, fc_roleid, false);
        _offline_roles.insert(std::make_pair(fc_roleid,fc_chatid));
        //_updating_role_session.erase(fc_roleid);
    }


    void Send(int zoneid, const Protocol &protocol)
    {
        //CentralDeliveryServerMan::GetInstance()->DispatchProtocol(zoneid, protocol)
    }

    void GetChatList(int fc_roleid)
    {
        role_iterator it = _roles.find(fc_roleid);
        if(it == _roles.end()) return;
        FC_Role &role = it->second;
        role_chats_iterator it2 = role.own_chats.begin();
        for(; it2!=role.own_chats.end(); ++it2)
        {
            chat_iterator it3 = _chats.find(it2->first);
            if(it3 == _chats.end())
            {
                //logerr
                continue;
            }
            it3->second.chat_name;
            //it3->second.introduction;//maybe
            it2->second;//ignore_msg
        }
        it2 = role.join_chats.begin();
        for(; it2!=role.join_chats.end(); ++it2)
        {
            chat_iterator it3 = _chats.find(it2->first);
            if(it3 == _chats.end())
            {
                //logerr
                continue;
            }
            it3->second.chat_name;
            //it3->second.introduction;//maybe
            it2->second;//ignore_msg
        }
        //Send
    }
    void GetChatMember(int fc_roleid, int fc_chatid)
    {
        if(_roles.find(fc_roleid) == _roles.end()) return;
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        //maybe need it->second.introduction
        std::map<int,FC_ChatRole> &members = it->second.members;
        if(members.find(fc_roleid) == members.end()) return;
        chat_members_iterator it2 = members.begin();
        for(; it2!=members.end(); ++it2)
        {
            it2->second.name;
            it2->second.zoneid;
            //...
        }
        it->second.introduction;
        //Send
    }
    void GetMemberMate(int fc_roleid, int fc_chatid, int mate_roleid)
    {
        if(_roles.find(fc_roleid) == _roles.end()) return;
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        std::map<int,FC_ChatRole> &members = it->second.members;
        if(members.find(fc_roleid) == members.end()) return;
        chat_members_iterator it2 = members.find(mate_roleid);
        if(it2 == members.end()) return;
        it2->second.name;
        it2->second.zoneid;
        //...
        //Send
    }

    void OwnerGetChatInfo()
    {
        int fc_roleid;
        int fc_chatid;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || it->second.owner != fc_roleid) return;
        it->second.can_search;
        it->second.confirmation;
        it->second.get_capacity_limit();
        //Send
    }

    void CreateChat(int fc_roleid)
    {
        Octets chat_name;
        ChatRoleInfo info;

        bool save_role = false;
        role_iterator it;
        if(!fc_roleid)
        {
            fc_roleid = AllocRoleid();
            it = _roles.insert(std::make_pair(fc_roleid,FC_Role(info))).first;
            save_role = true;
            //Send fc_roleid
        }
        else
        {
            it = _roles.find(fc_roleid);
            if(it == _roles.end())
            {
                //logerr
                return;
            }
        }

        FC_Role &role = it->second;
        if(role.own_chats.size() < role.get_own_limit())
        {
            int fc_chatid = AllocChatid();
            role.own_chats.insert(std::make_pair(fc_chatid,false));
            DoChatSave(fc_chatid,_chats.insert(std::make_pair(fc_chatid,FC_Chat(chat_name,fc_roleid))).first->second);
            save_role = true;
            //Send fc_chatid
        }
        DoNextidSave();
        if(save_role) DoRoleSave(fc_roleid,role);
    }

    void ChangeChatName()
    {
        int fc_roleid;
        int fc_chatid;

        Octets name;
        Octets introduction;

        bool name_changed = false;
        bool intr_changed = false;
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid != it->second.owner) return;
        FC_Chat &chat = it->second;
        if(!name.empty()) name_changed = true;
        if(!introduction.empty()) intr_changed = true;
        if(name_changed || intr_changed)
        {
            if(name_changed)
            {
                std::pair<std::multimap<Octets,int>::iterator,std::multimap<Octets,int>::iterator> ret = _name_map.equal_range(chat.chat_name);
                for(std::multimap<Octets,int>::iterator it=ret.first;it!=ret.second;++it)
                {
                    if(it->second == fc_chatid)
                    {
                        _name_map.erase(it);
                        break;
                    }
                }
                _name_map.insert(std::make_pair(name,fc_chatid));
                chat.chat_name = name;
            }
            if(intr_changed) chat.introduction = introduction;
            for(chat_members_iterator it2=chat.members.begin(); it2!=chat.members.end(); ++it2)
            {
                if(it2->second.online)
                {
                    //Send to it2->second.zoneid it2->second.roleid
                }
            }
            DoChatSave(fc_chatid,chat);
        }
    }
    void ChangeChatSetting()
    {
        int fc_roleid;
        int fc_chatid;

        char confirmation;
        bool can_search;

        bool save_chat = false;
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid != it->second.owner) return;
        FC_Chat &chat = it->second;
        if(confirmation && confirmation != chat.confirmation)
        {
            save_chat = true;
            chat.confirmation = confirmation;
        }
        if(can_search != chat.can_search)
        {
            save_chat = true;
            chat.can_search = can_search;
        }
        if(save_chat) DoChatSave(fc_chatid,chat);
    }

    void ChangeOwner()
    {
        int transferee;
        bool transferee_online;
        bool from_transferer;
        //确认transferee在线才能操作,其实只需要能在_roles中找得到就可以,但由于不能准确判断其是否在线,只好统一为必须在线
        if(from_transferer)
        {
            role_iterator it = _roles.find(transferee);
            if(it == _roles.end())
            {
                //Send back err
            }
            else
            {
                //Send to it->second.zoneid it->second.roleid
            }
        }
        else
        {
            if(transferee_online) DoChangeOwner();
            else
            {
                //Send back err
            }
        }
    }
    void DoChangeOwner()
    {
        int fc_roleid;
        int fc_chatid;
        int transferee;
        bool transferee_online;
        bool from_transferer;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid != it->second.owner) return;
        role_iterator it2 = _roles.find(transferee);
        if(it2 == _roles.end()) return;
        role_iterator it3 = _roles.find(fc_roleid);
        if(it3 == _roles.end()) return;

        FC_Role &to = it2->second;
        FC_Role &from = it3->second;
        if(to.own_chats.size() >= to.get_own_limit() || from.join_chats.size() >= from.get_join_limit())
            return;

        role_chats_iterator it4 = to.join_chats.find(fc_chatid);
        role_chats_iterator it5 = from.own_chats.find(fc_chatid);
        if(it4 == to.join_chats.end() || it5 == from.own_chats.end()) return;

        FC_Chat &chat = it->second;
        chat.owner = transferee;
        to.own_chats.insert(std::make_pair(fc_chatid,it4->second));
        to.join_chats.erase(fc_chatid);
        from.join_chats.insert(std::make_pair(fc_chatid,it5->second));
        from.own_chats.erase(fc_chatid);

        for(chat_members_iterator it6=chat.members.begin(); it6!=chat.members.end(); ++it6)
        {
            if(it6->second.online)
            {
                //Send to it6->second.zoneid it6->second.roleid
            }
        }

        DoRoleSave(fc_roleid,from);
        DoChatSave(fc_chatid,chat);
        DoRoleSave(transferee,to);
    }

    void KickMember()
    {
        int fc_roleid;
        int fc_chatid;
        int kick_role;//may not online

        if(fc_roleid == kick_role) return;
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid != it->second.owner) return;
        std::map<int,FC_ChatRole> members = it->second.members;
        chat_members_iterator it2 = members.find(kick_role);
        if(it2 == members.end()) return;
        role_iterator it3 = _roles.find(kick_role);//!!!!!!kick_role may be not exist!!!
        //if(it3 == _roles.end()) return;
        bool target_exist = it3 != _roles.end();
        it->second.members.erase(kick_role);
        if(target_exist) it3->second.join_chats.erase(fc_chatid);

        chat_members_iterator it4 = it->second.members.begin();
        for(; it4!=it->second.members.end(); ++it4)
        {
            //tell members
            it4->second.zoneid;
            it4->second.roleid;
        }

        if(target_exist) DoRoleSave(kick_role,it3->second);
        else //Send to db to correct kick_role fc_chatid
        DoChatMemberSave(fc_chatid,members);
    }

    void LeaveChat()
    {
        int fc_roleid;
        int fc_chatid;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid == it->second.owner) return;
        role_iterator it2 = _roles.find(fc_roleid);
        if(it2 == _roles.end()) return;

        std::map<int,FC_ChatRole> &members = it->second.members;
        members.erase(fc_roleid);
        it2->second.join_chats.erase(fc_chatid);

        for(chat_members_iterator it3=members.begin(); it3!=members.end(); ++it3)
        {
            if(it3->second.online)
            {
                //Send to it3->second.zoneid it3->second.roleid
            }
        }

        DoRoleSave(fc_roleid,it2->second);
        DoChatMemberSave(fc_chatid,members);
    }

    void DisbandChat(int fc_roleid, int fc_chatid)
    {
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || fc_roleid != it->second.owner) return;

        FC_Chat &chat = it->second;
        chat.is_delete = true;
        for(chat_members_iterator it2=chat.members.begin(); it2!=chat.members.end(); ++it2)
        {
            role_iterator it3 = _roles.find(it2->first);
            if(it3 == _roles.end()) continue;
            if(chat.owner == it2->first)
            {
                it3->second.own_chats.erase(fc_chatid);
            }
            else
            {
                it3->second.join_chats.erase(fc_chatid);
            }
            //Send it3->second.zoneid it3->second.roleid
            DoRoleSave(it2->first, it3->second);
        }
        DoChatSave(fc_chatid, it->second);
    }

    void IgnoreMsg()
    {
        int fc_roleid;
        int fc_chatid;
        bool chat_type;//mark own or join
        bool ignore;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        chat_members_iterator it2 = it->second.members.find(fc_roleid);
        if(it2 == it->second.members.end()) return;
        role_iterator it3 = _roles.find(fc_roleid);
        if(it3 == _roles.end()) return;

        if(chat_type == 1)
        {
            role_chats_iterator it4 = it3->second.own_chats.find(fc_chatid);
            if(it4 == it3->second.own_chats.end()) return;
            it4->second = ignore;
        }
        else if(chat_type == 2)
        {
            role_chats_iterator it4 = it3->second.join_chats.find(fc_chatid);
            if(it4 == it3->second.join_chats.end()) return;
            it4->second = ignore;
        }
        else
        {
            return;
        }
        it2->second.ignore_msg = ignore;
        DoRoleSave(fc_roleid,it3->second);
        DoChatMemberSave(fc_chatid,it->second.members);
    }

    void Talk()
    {
        int fc_roleid;
        int fc_chatid;
        char vip;
        Octets msg;

        //send to everyone except sender
        //not offline   not ignore
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;

        std::map<int,FC_ChatRole> &members = it->second.members;
        if(members.find(fc_roleid) == members.end()) return;
        for(chat_members_iterator it2=members.begin(); it2!=members.end(); ++it2)
        {
            if(it2->first == fc_roleid) continue;
            if(!it2->second.online) continue;
            if(it2->second.ignore_msg) continue;
            //Send to it2->second.zoneid and it2->second.roleid
        }
    }

    void SearchByChatid()
    {
        int fc_chatid;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || !it->second.can_search) return;
        it->second.chat_name;
        it->second.introduction;
        it->second.owner;
    }
    void SearchByChatname()
    {
        Octets chat_name;

        std::pair<std::multimap<Octets,int>::iterator,std::multimap<Octets,int>::iterator> ret = _name_map.equal_range(chat_name);
        for(std::multimap<Octets,int>::iterator it=ret.first; it!=ret.second; ++it)
        {
            chat_iterator it2 = _chats.find(it->second);
            if(it2 == _chats.end() || !it2->second.can_search) continue;
        }
    }
    

    void Invite()
    {
        int fc_roleid;
        int fc_chatid;
        int to_fc_roleid;
        ChatRoleInfo info;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        bool is_owner = fc_roleid == it->second.owner;
        if(!is_owner)
        {
            chat_members_iterator it2 = it->second.members.find(fc_roleid);
            if(it2 == it->second.members.end()) return;
        }
        if(to_fc_roleid == 0)
        {
            to_fc_roleid = AllocRoleid();
            _roles.insert(std::make_pair(to_fc_roleid,FC_Role(info)));//his name?
            //tell him
            //require a player logon
        }
        else
        {
            //it->second.invitee.insert(std::make_pair(AllocInviteid(),FC_ChatInvitee()));
            if(is_owner)
            {
            }
            else
            {
                if(it->second.confirmation == 8)
                {
                }
                else
                {
                    if(it->second.confirmation)
                    {
                        //send to invitee
                    }
                    else
                    {
                        //if owner is exist in _roles
                        //send to owner
                    }
                }
            }
        }
    }

    void Apply1()
    {
        int fc_roleid;
        int fc_chatid;
        int zoneid;
        int factionid;
        int familyid;
        ChatRoleInfo info;

        if(fc_roleid == 0)
        {
            fc_roleid = AllocRoleid();
            DoRoleSave(fc_roleid,_roles.insert(std::make_pair(fc_roleid,FC_Role(info))).first->second);
        }
        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end()) return;
        if(it->second.applicant.find(fc_roleid) != it->second.applicant.end()) return;
        //Send to onwer
    }
    void Apply2()
    {
    }
    void DisposeApply()
    {
        int fc_roleid;
        int fc_chatid;
        int applyer;
        bool permit;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || it->second.owner != fc_roleid) return;
        std::map<int,FC_ChatRole>::iterator it2 = it->second.applicant.find(applyer);
        if(it2 == it->second.applicant.end()) return;
        if(permit)
        {
            role_iterator it3 = _roles.find(applyer);
            if(it3 == _roles.end()) return;
            it3->second.join_chats.insert(std::make_pair(fc_chatid,false));
            it->second.members.insert(std::make_pair(applyer,it2->second));
        }
        else
        {
        }
        it->second.applicant.erase(it2);
    }

    void AddChatCapacity()
    {
        int fc_roleid;
        int fc_chatid;
        short add_capacity;

        chat_iterator it = _chats.find(fc_chatid);
        if(it == _chats.end() || it->second.owner != fc_roleid) return;
        if(it->second.get_capacity_limit() + add_capacity > MAX_CAPACITY_PER_CHAT) return;
        it->second.capacity_addon += add_capacity;
    }
    void AddOwnLimit()
    {
        int fc_roleid;
        int fc_chatid;
        short add;

        role_iterator it = _roles.find(fc_roleid);
        if(it == _roles.end()) return;
        if(it->second.get_own_limit() + add > MAX_OWN_LIMIT_PER_ROLE) return;
        it->second.own_chats_addon += add;
    }
    void AddJoinLimit()
    {
        int fc_roleid;
        int fc_chatid;
        short add;

        role_iterator it = _roles.find(fc_roleid);
        if(it == _roles.end()) return;
        if(it->second.get_join_limit() + add > MAX_JOIN_LIMIT_PER_ROLE) return;
        it->second.join_chats_addon += add;
    }

    virtual bool Update()
    {
        LOG_TRACE("statistics:nextroleid=%d,nextchatid=%d,roles=%d,chats=%d,tosavechats=%d",_next_roleid,_next_chatid,_roles.size(),_chats.size(),_tosave_chats.size());

        static std::set<int> steps;
        static int ticks;
        if(1)//2:00 AM
        {
            steps.swap(_tosave_chats);
            ticks = _tosave_chats.size() / 800;
            if(ticks > 10) ticks = 10;
        }
        if(2)//2:01 AM
        {
            if(!steps.empty())
            {
                std::vector<int> step;
                std::set<int>::iterator it = steps.begin();
                for(int i=0; i<ticks && it!=steps.end(); ++i)
                {
                    step.push_back(*it);
                    steps.erase(it++);
                }
                save_chats_step(step);
            }
        }
        return true;
    }
};


struct LoadRoleInfo
{
    virtual void Process()=0;
};
class Hook_OnPlayerOn:public LoadRoleInfo
{
    public:
    int i;
    void Process()
    {
        i = 8;
        printf("%d\n",i);
        //update
    }
};
class LoadChatInfo
{
    virtual void Process()=0;
};
class LoadChatMembers
{
    virtual void Process()=0;
};
void fun(LoadRoleInfo &inf)
{
    inf.Process();
}

class rpc
{
    public:
        LoadRoleInfo &info;
        rpc(LoadRoleInfo &i):info(i){}
        void fun()
        {
            info.Process();
        }

};

struct FC_Role_vector
{
    Octets name;
    int zoneid;
    int roleid;//update right now
    short level;
    char cultivation;
    char occupation;
    short own_chats_addon;//move to origial
    short join_chats_addon;
    time_t last_active_time;
    std::vector<int> own_chats;//key is fc_chatid,value is ignore_msg
    std::vector<bool> join_chats;//key is fc_chatid,value is ignore_msg
};
int main()
{
Hook_OnPlayerOn info;
rpc r(info);
r.fun();
return 0;

    FC_Server server;

    FC_Role role;
    //role.name = "1234567890123456";
    //role.name.begin();
    /*
    for(int i=0;i<10;++i)
        role.own_chats.insert(std::make_pair(i,true));
    for(int i=0;i<35;++i)
        role.join_chats.insert(std::make_pair(i,true));
    for(int i=0;i<1000000;++i)
        server._roles.insert(std::make_pair(i,role));
    puts("end");
    for(;;) sleep(10);
        */

    std::map<int,FC_Role_vector> rvserver;
    FC_Role_vector rv;
    rv.name = "1234567890123456";
    rv.name.begin();
    for(int i=0;i<45;++i)
        rv.own_chats.push_back(i);
    for(int i=0;i<45;++i)
        rv.join_chats.push_back(true);
    for(int i=0;i<1000000;++i)
        rvserver.insert(std::make_pair(i,rv));
    puts("end");
    for(;;) sleep(10);
    std::pair<int,bool> b(4,true);
    printf("%d\n",sizeof(b));
    return 0;
    Octets name("12345677654321");
    FC_ChatRole chatrole;
    for(int i=0;i<10000;++i)
    {
        FC_Chat chat(name,i);
        chat.chat_name.begin();
        chat.introduction = "123456789012345678901234567890123456789012345678901234567890";
        chat.introduction.begin();
        //if(i<10000)
        for(int j=0;j<75;++j) chat.members.insert(std::make_pair(j,chatrole));
        server._chats.insert(std::make_pair(i,chat));
    }
    puts("end");
    for(;;) sleep(10);
    return 0;
}


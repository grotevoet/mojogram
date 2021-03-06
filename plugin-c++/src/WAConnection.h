/*
 * WAConnection.h
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#ifndef WACONNECTION_H_
#define WACONNECTION_H_

#include <string>
#include <time.h>
#include <map>
#include "WAException.h"
#include "FMessage.h"
#include "WALogin.h"
#include "utilities.h"
#include "BinTreeNodeReader.h"
#include "BinTreeNodeWriter.h"
#include <SDL.h>

class WALogin;
class KeyStream;
class BinTreeNodeReader;

class WAListener {
public:
	virtual void onMessageForMe(FMessage* paramFMessage, bool paramBoolean) throw (WAException)=0;
	virtual void onMessageStatusUpdate(FMessage* paramFMessage)=0;
	virtual void onMessageError(FMessage* message, int paramInt)=0;
	virtual void onPing(const std::string& paramString) throw (WAException)=0;
	virtual void onPingResponseReceived()=0;
	virtual void onAvailable(const std::string& paramString, bool paramBoolean)=0;
	virtual void onClientConfigReceived(const std::string& paramString)=0;
	virtual void onLastSeen(const std::string& paramString1, int paramInt, std::string* paramString2)=0;
	virtual void onIsTyping(const std::string& paramString, bool paramBoolean)=0;
	virtual void onAccountChange(int paramInt, long paramLong)=0;
	virtual void onPrivacyBlockListAdd(const std::string& paramString)=0;
	virtual void onPrivacyBlockListClear()=0;
	virtual void onDirty(const std::map<string, string>& paramHashtable)=0;
	virtual void onDirtyResponse(int paramHashtable)=0;
	virtual void onRelayRequest(const std::string& paramString1, int paramInt, const std::string& paramString2)=0;
	virtual void onSendGetPictureIds(std::map<string, string>* ids)=0;
	virtual void onSendGetPicture(const std::string& jid, const std::vector<unsigned char>& data, const std::string& oldId, const std::string& newId)=0;
	virtual void onPictureChanged(const std::string& from, const std::string& author, bool set)=0;
	virtual void onDeleteAccount(bool result)=0;
	virtual void onMediaUploadRequest(const std::string& status, const std::string& msgId, const std::string& hash, const std::string& url, int resumeFrom)=0;
};

class WAGroupListener {
public:
	virtual void onGroupAddUser(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupRemoveUser(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupNewSubject(const std::string& from, const std::string& author, const std::string& newSubject, int paramInt)=0;
	virtual void onServerProperties(std::map<std::string, std::string>* nameValueMap)=0;
	virtual void onGroupCreated(const std::string& paramString1, const std::string& paramString2)=0;
	virtual void onGroupInfo(const std::string& paramString1, const std::string& paramString2, const std::string& paramString3, const std::string& paramString4, int paramInt1, int paramInt2)=0;
	virtual void onGroupInfoFromList(const std::string& paramString1, const std::string& paramString2, const std::string& paramString3, const std::string& paramString4, int paramInt1,
			int paramInt2)=0;
	virtual void onOwningGroups(const std::vector<string>& paramVector)=0;
	virtual void onSetSubject(const std::string& paramString)=0;
	virtual void onAddGroupParticipants(const std::string& paramString, const std::vector<string>& paramVector, int paramHashtable)=0;
	virtual void onRemoveGroupParticipants(const std::string& paramString, const std::vector<string>& paramVector, int paramHashtable)=0;
	virtual void onGetParticipants(const std::string& gjid, const std::vector<string>& participants)=0;
	virtual void onParticipatingGroups(const std::vector<string>& paramVector)=0;
	virtual void onLeaveGroup(const std::string& paramString)=0;
};

class MessageStore {
public:
	MessageStore();

	virtual FMessage* get(Key* key);

	virtual ~MessageStore();
};

class GroupSetting {
public:
	std::string jid;
	bool enabled;
	time_t muteExpiry;

	GroupSetting() {
		enabled = true;
		jid = "";
		muteExpiry = 0;
	}
};

class WAConnection {

	class IqResultHandler {
	protected:
		WAConnection* con;
	public:
		IqResultHandler(WAConnection* con) {
			this->con = con;
		}
		virtual void parse(ProtocolTreeNode* paramProtocolTreeNode, const std::string& paramString) throw (WAException)=0;
		void error(ProtocolTreeNode* node, int code) {
			_LOGDATA("WAConnection: error node %s: code = %d", node->getAttributeValue("id")->c_str(), code);
		}
		void error(ProtocolTreeNode* node) throw (WAException) {
			std::vector<ProtocolTreeNode*>* nodes = node->getAllChildren("error");
			for (size_t i = 0; i < nodes->size(); i++) {
				ProtocolTreeNode* errorNode = (*nodes)[i];
				if (errorNode != NULL) {
					std::string* errorCodeString = errorNode->getAttributeValue("code");
					if (errorCodeString != NULL) {
						int errorCode = atoi(errorCodeString->c_str());
						error(node, errorCode);
					}
				}
			}
			delete nodes;
		}

		virtual ~IqResultHandler() {
		}

	};

	class IqResultPingHandler: public IqResultHandler {
	public:
		IqResultPingHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->event_handler != NULL)
				this->con->event_handler->onPingResponseReceived();
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->event_handler != NULL)
				this->con->event_handler->onPingResponseReceived();
		}
	};

	class IqResultGetGroupsHandler: public IqResultHandler {
	private:
		std::string type;
	public:
		IqResultGetGroupsHandler(WAConnection* con, const std::string& type) :
				IqResultHandler(con) {
			this->type = type;
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector < std::string > groups;
			this->con->readGroupList(node, groups);
			if (this->con->group_event_handler != NULL) {
				if (this->type.compare("participating") == 0)
					this->con->group_event_handler->onParticipatingGroups(groups);
				else if (this->type.compare("owning") == 0)
					this->con->group_event_handler->onOwningGroups(groups);
			}
		}
	};

	class IqResultServerPropertiesHandler: public IqResultHandler {
	public:
		IqResultServerPropertiesHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector<ProtocolTreeNode*>* nodes = node->getAllChildren("prop");
			std::map < std::string, std::string > nameValueMap;
			for (size_t i = 0; i < nodes->size(); i++) {
				ProtocolTreeNode* propNode = (*nodes)[i];
				std::string* nameAttr = propNode->getAttributeValue("name");
				std::string* valueAttr = propNode->getAttributeValue("value");
				nameValueMap[*nameAttr] = *valueAttr;
			}
			delete nodes;
			if (this->con->group_event_handler != NULL)
				this->con->group_event_handler->onServerProperties(&nameValueMap);
		}
	};

	class IqResultPrivayListHandler: public IqResultHandler {
	public:
		IqResultPrivayListHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* queryNode = node->getChild(0);
			ProtocolTreeNode::require(queryNode, "query");
			ProtocolTreeNode* listNode = queryNode->getChild(0);
			ProtocolTreeNode::require(listNode, "list");
			if (this->con->event_handler != NULL)
				this->con->event_handler->onPrivacyBlockListClear();
			if (listNode->children != NULL) {
				for (size_t i = 0; i < listNode->children->size(); i++) {
					ProtocolTreeNode* itemNode = (*listNode->children)[i];
					ProtocolTreeNode::require(itemNode, "item");
					if (itemNode->getAttributeValue("type")->compare("jid") == 0) {
						std::string* jid = itemNode->getAttributeValue("value");
						if (jid != NULL && this->con->event_handler != NULL)
							this->con->event_handler->onPrivacyBlockListAdd(*jid);
					}
				}
			}
		}
	};

	class IqResultGetGroupInfoHandler: public IqResultHandler {
	public:
		IqResultGetGroupInfoHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* groupNode = node->getChild(0);
			ProtocolTreeNode::require(groupNode, "group");
			// std::string* gid = groupNode->getAttributeValue("id");
			std::string* owner = groupNode->getAttributeValue("owner");
			std::string* subject = groupNode->getAttributeValue("subject");
			std::string* subject_t = groupNode->getAttributeValue("s_t");
			std::string* subject_owner = groupNode->getAttributeValue("s_o");
			std::string* creation = groupNode->getAttributeValue("creation");
			if (this->con->group_event_handler != NULL)
				this->con->group_event_handler->onGroupInfo(from, *owner, *subject, *subject_owner, atoi(subject_t->c_str()), atoi(creation->c_str()));
		}
	};

	class IqResultGetGroupParticipantsHandler: public IqResultHandler {
	public:
		IqResultGetGroupParticipantsHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::vector < std::string > participants;
			this->con->readAttributeList(node, participants, "participant", "jid");
			if (this->con->group_event_handler != NULL)
				this->con->group_event_handler->onGetParticipants(from, participants);
		}
	};

	class IqResultCreateGroupChatHandler: public IqResultHandler {
	public:
		IqResultCreateGroupChatHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* groupNode = node->getChild(0);
			ProtocolTreeNode::require(groupNode, "group");
			std::string* groupId = groupNode->getAttributeValue("id");
			if (groupId != NULL && con->group_event_handler != NULL)
				this->con->group_event_handler->onGroupCreated(from, *groupId);
		}
	};

	class IqResultQueryLastOnlineHandler: public IqResultHandler {
	public:
		IqResultQueryLastOnlineHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* firstChild = node->getChild(0);
			ProtocolTreeNode::require(firstChild, "query");
			std::string* seconds = firstChild->getAttributeValue("seconds");
			std::string* status = NULL;
			status = firstChild->getDataAsString();
			if (seconds != NULL && !from.empty()) {
				if (this->con->event_handler != NULL)
					this->con->event_handler->onLastSeen(from, atoi(seconds->c_str()), status);
			}
			delete status;
		}
	};

	class IqResultGetPhotoHandler: public IqResultHandler {
	private:
		std::string jid;
		std::string oldId;
		std::string newId;
	public:
		IqResultGetPhotoHandler(WAConnection* con, const std::string& jid, const std::string& oldId, const std::string& newId) :
				IqResultHandler(con) {
			this->jid = jid;
			this->oldId = oldId;
			this->newId = newId;
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			std::string* attributeValue = node->getAttributeValue("type");

			if ((attributeValue != NULL) && (attributeValue->compare("result") == 0) && (this->con->event_handler != NULL)) {
				std::vector<ProtocolTreeNode*>* children = node->getAllChildren("picture");
				for (int i = 0; i < children->size(); i++) {
					ProtocolTreeNode* current = (*children)[i];
					std::string* id = current->getAttributeValue("id");
					if ((id != NULL) && (current->data != NULL) && (current->data->size() > 0)) {
						if (current->data != NULL) {
							this->con->event_handler->onSendGetPicture(this->jid, *current->data, this->oldId, this->newId);
						}
						break;
					}
				}
				delete children;
			}
		}
		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->event_handler != NULL) {
				std::vector<unsigned char> v;
				this->con->event_handler->onSendGetPicture("error", v, "", "");
			}
		}
	};

	class IqResultSetPhotoHandler: public IqResultHandler {
	private:
		std::string jid;
	public:
		IqResultSetPhotoHandler(WAConnection* con, const std::string& jid) :
				IqResultHandler(con) {
			this->jid = jid;
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->event_handler != NULL) {
				std::string* photoId = NULL;
				ProtocolTreeNode* child = node->getChild("picture");
				if (child != NULL) {
					this->con->event_handler->onPictureChanged(this->jid, "", true);
				} else {
					this->con->event_handler->onPictureChanged(this->jid, "", false);
				}
			}
		}
	};

	class IqResultGetPictureIdsHandler: public IqResultHandler {
	public:
		IqResultGetPictureIdsHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			// _LOGDATA("onGetPhotoIds %s", node->toString().c_str());
			ProtocolTreeNode* groupNode = node->getChild("list");
			std::vector<ProtocolTreeNode*>* children = groupNode->getAllChildren("user");
			std::map < std::string, std::string > ids;
			for (int i = 0; i < children->size(); i++) {
				std::string* jid = (*children)[i]->getAttributeValue("jid");
				std::string* id = (*children)[i]->getAttributeValue("id");
				if (jid != NULL) {
					ids[*jid] = (id == NULL ? "" : *id);
				}
			}
			delete children;

			if (this->con->event_handler != NULL) {
				this->con->event_handler->onSendGetPictureIds(&ids);
			}
		}
	};

	class IqResultSendDeleteAccount: public IqResultHandler {
	public:
		IqResultSendDeleteAccount(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			if (this->con->event_handler != NULL) {
				this->con->event_handler->onDeleteAccount(true);
			}
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			if (this->con->event_handler != NULL) {
				this->con->event_handler->onDeleteAccount(false);
			}
		}
	};

	class IqResultClearDirtyHandler: public IqResultHandler {
	public:
		IqResultClearDirtyHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
		}
	};

	class IqSendClientConfigHandler: public IqResultHandler {
	public:
		IqSendClientConfigHandler(WAConnection* con) :
				IqResultHandler(con) {
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			_LOGDATA("Clientconfig response %s", node->toString().c_str());
		}

		void error(ProtocolTreeNode* node) throw (WAException) {
			_LOGDATA("Clientconfig response error %s", node->toString().c_str());
		}
	};

	class IqResultRequestUploadHandler: public IqResultHandler {
	private:
		std::string hash;
		std::string msgId;
		int size;
	public:
		IqResultRequestUploadHandler(WAConnection* con, const std::string& hash, const std::string& msgId, int size) :
				IqResultHandler(con) {
			this->hash = hash;
			this->msgId = msgId;
			this->size = size;
		}
		virtual void parse(ProtocolTreeNode* node, const std::string& from) throw (WAException) {
			ProtocolTreeNode* mediaNode = node->getChild("media");
			if (mediaNode != NULL) {
				std::string* url = mediaNode->getAttributeValue("url");
				std::string* resumeFrom = mediaNode->getAttributeValue("resume");
				int resumeFromInt = 0;
				if (resumeFrom != NULL) {
					resumeFromInt = atoi(resumeFrom->c_str());
				}
				if (url != NULL) {
					this->con->event_handler->onMediaUploadRequest("success", this->msgId, this->hash, *url, resumeFromInt);
				} else {
					this->con->event_handler->onMediaUploadRequest("failed", this->msgId, this->hash, "", -1);
				}
			} else {
				ProtocolTreeNode* duplicateNode = node->getChild("duplicate");

				std::string* url = duplicateNode->getAttributeValue("url");

				if (duplicateNode != NULL) {
					this->con->event_handler->onMediaUploadRequest("duplicate", this->msgId, this->hash, *url, this->size);
				} else {
					this->con->event_handler->onMediaUploadRequest("failed", this->msgId, this->hash, "", -1);
				}
			}
		}
	};

private:
	WALogin* login;
	BinTreeNodeReader* in;
	BinTreeNodeWriter* out;
	WAListener* event_handler;
	WAGroupListener* group_event_handler;
	bool verbose;
	int iqid;
	std::map<string, IqResultHandler*> pending_server_requests;
	SDL_mutex* mutex;

	// std::<string, FMessage* > message_store;

	void init(WAListener* event_handler, WAGroupListener* group_event_handler);
	void sendMessageWithMedia(FMessage* message) throw (WAException);
	void sendMessageWithBody(FMessage* message) throw (WAException);
	void sendBroadcastMessageWithBody(const std::vector<std::string>& jids, FMessage* message) throw (WAException);
	void sendBroadcastMessageWithMedia(const std::vector<std::string>& jids, FMessage* message) throw (WAException);
	ProtocolTreeNode* getMediaNode(FMessage* message) throw (WAException);
	std::map<string, string>* parseCategories(ProtocolTreeNode* node) throw (WAException);
	void parseMessageInitialTagAlreadyChecked(ProtocolTreeNode* node) throw (WAException);
	ProtocolTreeNode* getReceiptAck(const std::string& to, const std::string& id, const std::string& receiptType) throw (WAException);
	std::string makeId(const std::string& prefix);
	void sendGetGroups(const std::string& id, const std::string& type) throw (WAException);
	void readGroupList(ProtocolTreeNode* node, std::vector<std::string>& groups) throw (WAException);
	std::string gidToGjid(const std::string& gid);
	void readAttributeList(ProtocolTreeNode* node, std::vector<std::string>& vector, const std::string& tag, const std::string& attribute) throw (WAException);
	void sendVerbParticipants(const std::string& gjid, const std::vector<std::string>& participants, const std::string& id, const std::string& inner_tag) throw (WAException);
	bool supportsReceiptAcks();
	static ProtocolTreeNode* getMessageNode(FMessage* message, ProtocolTreeNode* node1, ProtocolTreeNode* node2 = NULL);
	static ProtocolTreeNode* getSubjectMessage(const std::string& to, const std::string& id, ProtocolTreeNode* child) throw (WAException);
	std::vector<ProtocolTreeNode*>* processGroupSettings(const std::vector<GroupSetting>& gruops);

public:
	WAConnection(WAListener* event_handler = NULL, WAGroupListener* group_event_handler = NULL);
	virtual ~WAConnection();
	std::string jid;
	std::string fromm;
	int msg_id;
	int state;
	bool retry;
	time_t expire_date;
	int account_kind;
	time_t lastTreeRead;
	static const int DICTIONARY_LEN = 237;
	static const char* dictionary[];
	static MessageStore* message_store;
	KeyStream* inputKey;
	KeyStream* outputKey;

	static std::string removeResourceFromJid(const std::string& jid);

	WALogin* getLogin();
	void setLogin(WALogin* login);
	void setVerboseId(bool b);
	void sendMessage(FMessage* message) throw (WAException);
	void sendBroadcastMessage(const std::vector<std::string>& jids, FMessage* message) throw (WAException);
	void sendAvailableForChat() throw (WAException);
	bool read() throw (WAException);
	void sendNop() throw (WAException);
	void sendPing() throw (WAException);
	void sendQueryLastOnline(const std::string& jid) throw (WAException);
	void sendPong(const std::string& id) throw (WAException);
	void sendComposing(const std::string& to) throw (WAException);
	void sendActive() throw (WAException);
	void sendInactive() throw (WAException);
	void sendPaused(const std::string& to) throw (WAException);
	void sendSubjectReceived(const std::string& to, const std::string& id) throw (WAException);
	void sendMessageReceived(FMessage* message) throw (WAException);
	void sendDeliveredReceiptAck(const std::string& to, const std::string& id) throw (WAException);
	void sendVisibleReceiptAck(const std::string& to, const std::string& id) throw (WAException);
	void sendPresenceSubscriptionRequest(const std::string& to) throw (WAException);
	void sendClientConfig(const std::string& sound, const std::string& pushID, bool preview, const std::string& platform) throw (WAException);
	void sendClientConfig(const std::string& pushID, bool preview, const std::string& platform, bool defaultSettings, bool groupSettings, const std::vector<GroupSetting>& groups) throw (WAException);
	void sendClose() throw (WAException);
	void sendGetPrivacyList() throw (WAException);
	void sendGetServerProperties() throw (WAException);
	void sendGetGroups() throw (WAException);
	void sendGetOwningGroups() throw (WAException);
	void sendCreateGroupChat(const std::string& subject) throw (WAException);
	void sendEndGroupChat(const std::string& gjid) throw (WAException);
	void sendGetGroupInfo(const std::string& gjid) throw (WAException);
	void sendGetParticipants(const std::string& gjid) throw (WAException);
	void sendClearDirty(const std::string& category) throw (WAException);
	void sendLeaveGroup(const std::string& gjid) throw (WAException);
	void sendAddParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException);
	void sendRemoveParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException);
	void sendSetNewSubject(const std::string& gjid, const std::string& subject) throw (WAException);
	void sendStatusUpdate(std::string& status) throw (WAException);
	void sendGetPicture(const std::string& jid, const std::string& type, const std::string& oldId, const std::string& newId) throw (WAException);
	void sendGetPictureIds(const std::vector<std::string>& jids) throw (WAException);
	void sendSetPicture(const std::string& jid, std::vector<unsigned char>* data) throw (WAException);
	void sendNotificationReceived(const std::string& from, const std::string& id) throw (WAException);
	void sendDeleteAccount() throw (WAException);
	void sendRequestUpload(const std::string& msgId, const std::string& hash, int size, const std::string& type, const std::string& orighash) throw (WAException);
};

#endif /* WACONNECTION_H_ */

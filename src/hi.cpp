#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

#include "Geode/modify/PauseLayer.hpp"//left-button-menu
class $modify(PauseLayerExt, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (auto menu = querySelector("left-button-menu")) {
            if (auto item = CCMenuItemExt::createSpriteExtraWithFrameName(
                "GJ_chatBtn_001.png", 0.6f, [](void*) {
                    if (auto a = CCScene::get()->getChildByType<InfoLayer>(0))
                        a->onComment(a);
                }
            )) {
                menu->addChild(item);
            }
            if (auto item = CCMenuItemExt::createSpriteExtraWithFrameName(
                "GJ_chatBtn_001.png", 0.6f, [](void*) {
                    openInfoPopup(Mod::get());
                }
            )) {
                auto sprite = item->getNormalImage();
                if (auto sub = CCSprite::createWithSpriteFrameName(
                    "geode.loader/settings.png"
                )) {
                    sub->setPosition({ 34.000f, 14.000f });
					sub->setScale(0.775f);
                    sprite->addChild(sub);
                }
                menu->addChild(item);
            }
            menu->updateLayout();
        }
    }
};

#include "Geode/modify/PlayLayer.hpp"
class $modify(PlayLayerExt, PlayLayer) {
    class InfoLayerUpdater : public InfoLayer {
	public:
        void reload(float) {
			if (!this or !typeinfo_cast<InfoLayer*>(this)) return;
            //dont reset when dragging
            if (m_list) if (m_list->m_list)
                if (m_list->m_list->m_tableView)
                    if (m_list->m_list->m_tableView->m_touchDown) return;
            //reset
            this->onRefreshComments(nullptr);
        }
        void setup(float) {
			if (!this or !typeinfo_cast<InfoLayer*>(this)) return;
            this->setPosition(
                Mod::get()->getSavedValue<float>("window.pos.x", -202.000),
                Mod::get()->getSavedValue<float>("window.pos.y", -49.000f)
            );
            this->setScale(Mod::get()->getSettingValue<float>("scale"));
            this->setOpacity(0);
            this->setZOrder(999);
            for (auto a : m_mainLayer->getChildrenExt()) a->setVisible(
                typeinfo_cast<GJCommentListLayer*>(a)
            );
            m_reportBtn->setVisible(false);
            if (m_timeBtn) if (auto p = m_timeBtn->getParent()) p->setVisible(
                Mod::get()->getSettingValue<bool>("show-left-side-menu")
            );
            if (m_buttonMenu) m_buttonMenu->setVisible(
                Mod::get()->getSettingValue<bool>("show-menu")
            );
            if (m_list) {
                for (auto a : m_list->getChildrenExt()) a->setVisible(
                    typeinfo_cast<CustomListView*>(a)
                );
                findFirstChildRecursive<CCLayerColor>(
                    m_list, [](CCLayerColor* node) {
                        node->setColor(Mod::get()->getSettingValue<ccColor3B>("color"));
                        node->setOpacity(Mod::get()->getSettingValue<int>("opacity"));
                        return false;
                    }
                );
                if (m_list->m_list) if (auto scroll = m_list->m_list->m_tableView) {
                    scroll->setMouseEnabled(false);
                    scroll->m_disableVertical = false;
                    scroll->m_disableHorizontal = false;
                    auto content = scroll->m_contentLayer;
                    if (m_reportBtn->getPositionX() != 0) {
                        m_reportBtn->setPosition(content->getPosition());
                    }
                    if (scroll->m_touchDown) {
                        auto offset = this->getPosition() - ccpSub(
                            m_reportBtn->getPosition(),
                            content->getPosition()
                        );
                        Mod::get()->setSavedValue<float>("window.pos.x", offset.x);
                        Mod::get()->setSavedValue<float>("window.pos.y", offset.y);
                        content->setPosition(m_reportBtn->getPosition());
                    }
                }
            }
        }
    };
    void updateInfoLayer(float) { //meant to be called on scene
        if (auto info = m_uiLayer->getChildByID("comment-widget"_spr)) {
            static_cast<InfoLayer*>(info)->scheduleOnce(schedule_selector(InfoLayerUpdater::setup), 0.f);
		}
	}
    void updateInfoLayerLive(float) { //meant to be called on scene
		if (auto info = m_uiLayer->getChildByID("comment-widget"_spr)) {
            static_cast<InfoLayer*>(info)->scheduleOnce(schedule_selector(InfoLayerUpdater::reload), 1.f);
		}
	}
    virtual void setupHasCompleted() {
		PlayLayer::setupHasCompleted();
        if (!m_level || !m_uiLayer) return;
        if (m_level->m_levelType == GJLevelType::Editor || m_level->m_levelType == GJLevelType::Main) return;
        auto info = InfoLayer::create(m_level, nullptr, nullptr);
        info->setMouseEnabled(false);
        info->setTouchEnabled(false);
        info->setKeyboardEnabled(false);
        info->setID("comment-widget"_spr);
        m_uiLayer->addChild(info);
        this->schedule(schedule_selector(PlayLayerExt::updateInfoLayer));
        this->schedule(schedule_selector(PlayLayerExt::updateInfoLayerLive), 1.f);
    };
};
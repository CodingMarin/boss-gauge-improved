bool ZBossGaugeEffect::Draw(unsigned long int nTime, MDrawContext* pDC)
{
	MFont* pFont = ZGetCombatInterface()->GetGameFont();

	MUID uidBoss = MUID(0, 0);
	if (ZGetGame()->GetMatch()->IsQuestDrived())
	{
		uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();
	}
	else if(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		uidBoss = ((ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule())->GetBoss();
	}
	if (uidBoss == MUID(0,0)) return true;

	const float fShockDuration=	0.5f;
	const rvector ShockOffset=rvector(0,0,0);
	const rvector ShockVelocity=rvector(0,0,0);

	rvector offset = rvector(0.0f, 0.0f, 0.0f);

	float fElapsed = ZGetGame()->GetTime() - m_fLastTime;

	if (m_bShocked)
	{
		float fA=RandomNumber(0.0f, 1.0f)*2*pi;
		float fB=RandomNumber(0.0f, 1.0f)*2*pi;
		rvector velocity=rvector(cos(fA)*cos(fB), sin(fA)*sin(fB), 0.0f);

		float fPower=(ZGetGame()->GetTime() - m_fShockStartTime) / fShockDuration;
		if(fPower>1.f)
		{
			m_bShocked=false;
		}
		else
		{
			fPower=1.f-fPower;
			fPower=pow(fPower,1.5f);
			m_ShockVelocity = (RandomNumber(0.0f, 1.0f) * m_fShockPower * velocity);
			m_ShockOffset += fElapsed * m_ShockVelocity;
			offset = fPower * m_ShockOffset;
#ifdef DEBUG
			char text[256];
			sprintf(text, "%.3f, %.3f\n", offset.x, offset.y);
			OutputDebugString(text);
#endif
		}
	}

	m_fLastTime = ZGetGame()->GetTime();
	offset.z = 0.0f;


	bool ret = ZScreenEffect::DrawCustom(0, offset);

	// HP Gauge
	ZObject* pBoss = ZGetObjectManager()->GetObject(uidBoss);
	if ((pBoss) && (pBoss->IsNPC()))
	{
		ZActorBase* pBossActor = (ZActorBase*)pBoss;
		// In cases where the boss dies with AP remaining and HP becoming 0, AP is not drawn at all.
		int nMax = pBossActor->GetActualMaxHP()/* + pBossActor->GetActualMaxAP()*/;
		int nCurr = min(pBossActor->GetActualHP()/* + pBossActor->GetAP()*/, nMax);

		if ((m_nVisualValue < 0) || (m_nVisualValue > nCurr) || (nCurr - m_nVisualValue > 100))
		{
			m_nVisualValue = nCurr;
		}

		if (m_nVisualValue > 0)
		{
			char szText[256];

			float nHP = pBossActor->GetActualHP() + pBossActor->GetActualAP();
			float nMaxHP = pBossActor->GetActualMaxHP() + pBossActor->GetActualMaxAP();
			float percentage = nHP / nMaxHP;

			const int width = 433+1;
			const int height = 12;

			int x = (800 - width) * 0.5f;
			int y = 600 * 0.028f;

			float fGaugeWidth = width * (m_nVisualValue / (float)nMax);

			DWORD color = D3DCOLOR_ARGB(255, 0xBB, 0, 0);

			RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
			RGetDevice()->SetTexture(0, NULL);

			float fx = 183.0f / 800.0f/* + offset.x / 980.0f*/;
			float fy = 574.0f / 600.0f/* - offset.y / 720.0f*/;

			float posX = 540.0f / 800.f * (float)MGetWorkspaceWidth();
			float posY = 560.0f / 600.f * (float)MGetWorkspaceHeight();

			sprintf(szText, "BOSS: %.1f%%", percentage * 100.0f);

			DrawGauge(fx, fy, fGaugeWidth / 800.0f, 7.0f / 600.0f, 0.0f, color);

			pDC->SetColor(MCOLOR(0xFFFFFFFF));
			pDC->SetFont(pFont);
			pDC->Text(posX, posY, szText);
		}
	}


	return ret;
}


void ZScreenEffectManager::Draw(MDrawContext* pDC)
{
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	if (!ZGetCombatInterface()->GetObserverMode() && !ZGetCombatInterface()->IsSkupUIDraw())
	{
		RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		if (pTargetCharacter)
		{
			ZItem* pSelectedItem = pTargetCharacter->GetItems()->GetSelectedWeapon();

			if (pSelectedItem) {
				if (pSelectedItem->GetItemType() != MMIT_MELEE) {
					if (pSelectedItem->GetBulletCurrMagazine() <= 0) {
						if (pSelectedItem->isReloadable() == false) {
							m_bShowReload = false;
							m_bShowEmpty = true;
						}
						else {
							m_bShowReload = true;
							m_bShowEmpty = false;
						}
					}
					else {
						m_bShowReload = false;
						m_bShowEmpty = false;
					}
				}
				else {
					m_bShowReload = false;
					m_bShowEmpty = false;
				}
			}
		}

		if (m_bShowReload) {
			if (m_pReload)
			{
				m_pReload->Update();
				m_pReload->Draw(0);
			}
		}
		else if (m_bShowEmpty) {
			if (m_pEmpty)
			{
				m_pEmpty->Update();
				m_pEmpty->Draw(0);
			}
		}
	}

	LPDIRECT3DDEVICE9 pLPDIRECT3DDEVICE9 = RGetDevice();
	pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pLPDIRECT3DDEVICE9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	if (!ZGetGame()->IsReplay() || ZGetGame()->IsShowReplayInfo())
	{
		if (ZGetCombatInterface()->IsShowUI())
			DrawEffects(); // Draw combo animation

		// You have to manage the combo effect yourself.
		DrawCombo();
	}

	if (ZGetCombatInterface()->IsShowUI())
	{
		DrawQuestEffects(pDC); // Quest - K.O - image
		DrawDuelEffects();
		DrawTDMEffects();
	}
}

void ZScreenEffectManager::DrawQuestEffects(MDrawContext* pDC)
{
	bool isQuestDerived = ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType());
	bool isNewQuestDerived = ZGetGameTypeManager()->IsNewQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType());

	if (!isNewQuestDerived && !isQuestDerived)
		return;

	if (m_pBossHPPanel)
	{
		if(ZGetCombatInterface()->IsShowUI())
			m_pBossHPPanel->Draw(0, pDC);
	}

	// If it's a spec, KO and arrows are not drawn.
	if(!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		DrawKO();

		if (ZGetQuest()->IsRoundClear())
		{
			rvector to = ZGetQuest()->GetGameInfo()->GetPortalPos();
			DrawArrow(to);
		}
	}
}
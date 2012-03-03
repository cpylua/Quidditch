/*
	@author:	zombie.fml

	@history:	
	*	10/5/2009
		create file

	*	10/7/2009
		add handle related functions

	*	10/9/2009
		add gate related functions

		change some APIs' return type

	*	10/15/2009
		add Tc_CanSpell, Tc_CanBeSpelled, Tc_IsSpelling

		add hero and ball size

	*	10/18/2009
		add Tc_GetGameInfo, Tc_GetForbiddenArea and related structures

		change Tc_CanBeSpelled's signature

	*	10/19/2009
		change TC_Gate

		add TC_Ball.u.visible_time_left

	*	10/20/2009
		delete Tc_CanSpell and Tc_IsSpelling, merge them into TC_Hero. See
		TC_Hero.b_is_spelling and TC_Hero.b_can_spell

		add TC_Hero.spell_interval

	*	10/22/2009
		change Tc_GetEnemyGate's signature.
		It seems most people don't read the	comments when coding. I have no 
		choice but to change its signature. 

	*	10/24/2009
		change Tc_ChooseHero's signature, must specify a hero name.

		delete TC_Ball.handle_owner.

		make API compatible with Visaul C++ 6.0. I guess there's still someone 
		using Visual C++ 6.0.

	*	10/26/2009
		add moving_time_left, steps_before_next_snatch

	*	11/1/2009
		add TC_HERO_SPELL_DISATNCE

		add map size

		add snatch distances

	*	11/3/2009
		add MANA_RECHARGE

	*	11/10/2009
		add harry and voldemort

	*	11/13/2009
		change TC_ForbiddenArea, now it's a rectangle


	@description:
		defines client data structures and APIs
		

	@Remarks:
		With clients loaded as DLLs, several C/C++ run-time libraries can be
		present in a single address space. And this may lead to crashes.
		There's	an easy fix for this problem. When a module allocates memory, the 
		module must also free the allocated memory.
		As a result, none of the APIs allocates memory for callers. When an API 
		has a parameter that points to some places, it is assumed that the 
		caller has allocated the memory. P L E A S E keep this in mind.

		Never step out your AI when debugging if you are not familiar with
		multi-thread debugging. 

*/
#include <Windows.h>
#include <tchar.h>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

/*
	��ͼ��С
*/
#define TC_MAP_WIDTH	2048
#define TC_MAP_HEIGHT	768

/*
	Ӣ�۹�����Χ��
	����Ӣ���������ߵĳ���С�����ֵ˵���ڹ�����Χ��
*/
#define TC_HERO_SPELL_DISTANCE	512

/*
	���������Χ��
	�˺͹�������������������Χ��ʱ���п��ܡ������������
	�ܷ����������b_snatch_ghostball����
*/
#define TC_SNATCH_DISTANCE_GHOST	64

/*
	��������Χ��
	�˺ͷ������������������Χ��ʱ���п��ܡ�������������
	�ܷ���������b_snatch_goldball����
*/
#define TC_SNATCH_DISTANCE_GOLD		64

/*
	��������˵����ľ���С�����ֵʱ�ᱻ��������
*/
#define TC_COLLIDE_WITH_FREEBALL	64

/*
	��Զ�Ĵ������
*/
#define TC_PASSBALL_DISTANCE		512

/*
	Ӣ��ͼ���С
*/
#define TC_HERO_WIDTH	128
#define TC_HERO_HEIGHT	128

/*
	��ͼ���С
*/
#define TC_BALL_WIDTH	64
#define TC_BALL_HEIGHT	64

/*
	������С
*/
#define TC_GOLDBALL_WIDTH	128
#define TC_GOLDBALL_HEIGHT	128

/*
	��ħ�ٶ�
	ÿ֡��1��
*/
#define MANA_RECHARGE	1

/* 
	�����Ա��
*/
enum TC_Team_Member
{
	TC_MEMBER1,
	TC_MEMBER2,
};

/* 
	���ع��������

	export modifier
*/
#ifdef TC_CLIENT_EXPORTS
#define TC_CLIENT_API __declspec(dllexport)
#else
#define TC_CLIENT_API __declspec(dllimport)
#endif

/* 
	�����NULL��ʾ��Ч�ľ����

	���е�������ﶼ�þ����Ψһ��ʶ�����������нӿں�������һ�����
	�������͵Ĳ��������԰�����Ϊ��һ��Ψһ�ı�ʶ����
*/
typedef void* TC_Handle;

/*
	��ʾ�ٶȵĽṹ�壬
	vx��ˮƽ�ٶȣ�����Ϊ������vy�Ǵ�ֱ�ٶȣ�����Ϊ������
*/
typedef struct TC_Speed_tag
{
	int vx;
	int vy;
} TC_Speed, *PTC_Speed;

/*
	��ʾ����Ľṹ�壬
	��Ч��x��yֵӦ���ǷǸ�������
*/
typedef struct TC_Position_tag
{
	int x;		/* <0 if invalid */
	int y;		/* <0 if invalid */
} TC_Position, *PTC_Position;

/*
	���ܵ��������ͣ�
*/
enum TC_Hero_Type
{
	TC_HERO_RON,
	TC_HERO_HERMIONE,
	TC_HERO_MALFOY,
	TC_HERO_HARRY,
	TC_HERO_VOLDEMORT,
	TC_HERO_GINNY,
};

/*
	���ܵ������쳣״̬��
	�����쳣״̬��Ҫ��ָ���ĸ����͵���������򹥻��ˡ�
*/
enum TC_Abnormal_Type
{
	/* normal state */
	TC_SPELLED_BY_NONE,
	TC_SPELLING,

	/* abmormal states */
	TC_SPELLED_BY_RON,
	TC_SPELLED_BY_HERMIONE,
	TC_SPELLED_BY_MALFOY,
	TC_SPELLED_BY_HARRY,
	TC_SPELLED_BY_VOLDEMORT,
	TC_SPELLED_BY_GINNY,
	TC_SPELLED_BY_FREEBALL,
};

/*
	��ʾӢ�۵Ľṹ�壬
	Ӣ������Ļ������һ�����α�ʾ�ģ�TC_HERO_WIDTH��TC_HERO_HEIGHT�ֱ�����
	Ӣ�۵Ŀ�Ⱥ͸߶ȡ�

	type��ʾӢ�����ͣ���ο�TC_Hero_Type��ע�ͣ�

	speed��ʾӢ�۵�ǰ���ٶȣ�Ӣ�۱�����ʩ��������һ��ʱ�䣻

	pos��ʾӢ�۵ĵ�ǰλ�ã����������Ӣ�۵����Ͻǣ�

	max_blue��ʾӢ�۵����ħ��ֵ��

	curr_blue��ʾӢ�۵�ǰ��ħ��ֵ��

	spell_cost��ʾӢ��ʩ��һ��ħ��Ҫ�ķѶ���ħ��ֵ�����ܼ򵥵ĸ���
	spell_cost�Ƿ��curr_blueС���ж��Ƿ��ܷ�ħ������ȷ���������ж�
	Tc_Hero.b_can_spell

	spell_interval��ʾӢ�ۿ���ʩ�����ε���Сʱ������

	b_can_spell��ʾӢ���ܲ���ʩ��ħ��;

	b_is_spelling��ʾӢ���ǲ�������ʩ��ħ��;

	b_goldball�����true�ͱ�ʾӢ��ץס�˽�ɫ������ץס��ɫ������ζ��
	����������

	b_ghostball�����true�ͱ�ʾӢ��ץס�˹����

	b_snatch_ghostball�����true�ͱ�ʾӢ���ܹ�ȥ�������ע������������
	ʱ����һ���ɹ��������κ����������������˱�������һ����������

	b_snatch_goldball�����true�ͱ�ʾӢ���ܹ�ȥ����ɫ������

	abnormal_type��ʾӢ�۵�ǰ���쳣���ͣ�

	steps_before_next_snatch�����ָ�����ٹ����Ӣ�ۡ��п��ܡ��ܹ�����
	��Ϊ�ܷ�����Ҫ���Ǿ���Լ�����жϵ�ǰ�ܷ�������ʹ��b_snatch_ghostball��
	b_snatch_goldball��
	��������������֮����һ��ʱ��������ʱ�������ڲ�������
*/
typedef struct TC_Hero_tag
{
	TC_Hero_Type	type;
	TC_Speed		speed;
	TC_Position		pos;
	int				max_blue;
	int				curr_blue;
	int				spell_cost;
	int				spell_interval;
	bool			b_can_spell;
	bool			b_is_spelling;
	bool			b_goldball;	
	bool			b_ghostball;
	bool			b_snatch_ghostball;
	bool			b_snatch_goldball;
	TC_Abnormal_Type	abnormal_type;
	unsigned 		steps_before_next_snatch;
}TC_Hero, *PTC_Hero;

/*
	������ͣ���������

	��������ڵ�ǰ��Ļ�����ѡ������Ϊ�����յ�����������ֱ���˶���
	����ʱ�̲���������ʱ�䳤�Ȳ�����������������Ӣ�ۻ����ѣ��״̬��

	�������Ա���������ȥ������ѡ�ֳִ������Է����ż��ɵ÷�,
	ӵ�й����ʱ�˵��ƶ��ٶȻ������

	��ɫ�������Ա����������������������ͽ�������������ڵ�ǰ��Ļ����
	λ�ã�����ʱ��ǳ��̡�
*/
enum TC_Ball_Type
{
	TC_FREE_BALL,
	TC_GHOST_BALL,
	TC_GOLD_BALL,
};

/*
	��ʾһ��·��
*/
typedef struct TC_Path_tag
{
	TC_Position	pos_start;
	TC_Position	pos_end;
}TC_Path, *PTC_Path;

/*
	��ʾ��

	type��������ͣ���ο�TC_Ball_Type��ע�ͣ�

	speed����ǰ���ٶȣ��򲻿ɼ�ʱ��Ч��

	pos����ǰ��λ�ã�������Ͻ����꣩���򲻿ɼ�ʱ��Ч��

	b_visible�����true�ͱ�ʾ���ǿɼ��ģ�

	u.pathֻ�е�����������ʱ����Ч����������²���ʹ�������
	��ʾ��������˶�·����

	u.moving_time_leftֻ�е�����������ʱ����Ч����������²���ʹ�������
	��ʾ������ʣ��ɼ�ʱ�䡣

	u.targetֻ�е����ǹ����ʱ���ҹ���������ƶ����ٶȲ�Ϊ0������Ч����
	������²�Ӧ��ʹ�������
	��ʾ�����Ŀ�ĵ����ꡣ

	u.slow_down_steps_left����������ļ���ʱ�䡣

Microsoft Specific��
	Structure declarations can also be specified without a declarator when they 
	are members of another structure or union. The field names are promoted into
	the enclosing structure.
*/
#pragma warning(push)
#pragma warning(disable:201)
typedef struct TC_Ball_tag
{
	TC_Ball_Type		type;
	TC_Speed			speed;
	TC_Position			pos;
	bool				b_visible;
	union
	{
		/* free ball only */
		struct{
			TC_Path			path;		
			unsigned		moving_time_left;
		};

		/* ghost ball only */
		struct{
			TC_Position		target;	
		};

		/* gold ball only */
		struct{
			int				slow_down_steps_left;
		};
	} u;
}TC_Ball, *PTC_Ball;
#pragma warning(pop)

/*
	Ӣ���ƶ��ķ���
*/
enum TC_Direction
{
	TC_DIRECTION_BOTTOM,
	TC_DIRECTION_LEFTBOTTOM,
	TC_DIRECTION_LEFT,
	TC_DIRECTION_LEFTTOP,
	TC_DIRECTION_TOP,
	TC_DIRECTION_RIGHTTOP,
	TC_DIRECTION_RIGHT,
	TC_DIRECTION_RIGHTBOTTOM,
};

/*
	����λ��.

	y_upper �� y_lower����������֦�����ϵ���������

 'ֱ��X = x'
   /|\
    |
	
	|
	|
	|
+++++		--> 'ֱ��Y = y_upper'
|
|			��ֻ�ܴ�Y = y_upper �� Y = y_lower���е����򴩹������ߡ�
|
+++++		--> 'ֱ��Y = y_lower'
	|
	|
	|

	Ӣ�۴���Խ��������(X = x)����÷֡�ע�⣬����Ӣ�۱��붼������������
	����÷�
*/
typedef struct TC_Gate_tag
{
	int x;
	int y_upper;
	int y_lower;
} TC_Gate, *PTC_Gate;

/*
	����һ����������
*/
typedef struct TC_Rect_tag
{
	int top;
	int left;
	int bottom;
	int right;
} TC_Rect, *PTC_Rect;

/*
	����ʱ��ЧĿ�ĵر��뱣֤��
	�������򡿶����ڽ�����

	left����ߵĽ�����right���ұߵĽ�����
*/
typedef struct TC_ForbiddenArea_tag
{
	TC_Rect left;		
	TC_Rect right;
} TC_ForbiddenArea, *PTC_ForbiddenArea;

/*
	��Ϸ�����Ϣ

	score_self�����÷֣�

	score_enemy�Է��÷֣�

	game_time_left��Ϸʣ��ʱ�䣬��λ���롣
*/
typedef struct TC_GameInfo_tag
{
	unsigned score_self;
	unsigned score_enemy;

	unsigned game_time_left;	/* in ms */
} TC_GameInfo, *PTC_GameInfo;


/*

	��Ҫ˵�����ǣ�ע���п��ܻ���һЩ����һ���Ժ�����ԭ��Ϊ׼��
	���в��������ֶ���һ���ܺõ�ע�͡�

*/

/*
	���ö������ƣ��16���ַ���������β��'\0'��
	����̫���ᱻ�ضϡ�
	������UINCIODE�ַ���e.g. L"Team name"��

	�޷���ֵ
*/
extern "C" TC_CLIENT_API void  __stdcall Tc_SetTeamName(IN PCWSTR pszTeamName );



/*
	ѡ��Ӣ�����ͣ�

	p_indexָ���Ƕ����еĵڼ���Ա��
	p_Typeָ��Ӣ�۵����͡�

	�޷���ֵ��

*/
extern "C" TC_CLIENT_API void  __stdcall Tc_ChooseHero(IN TC_Team_Member index, IN TC_Hero_Type Type, IN PCWSTR pszHeroName);



/*
	�ƶ���

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	directionָ������

	����ɹ�������true�����򷵻�false��
*/
TC_CLIENT_API bool  __stdcall Tc_Move(IN TC_Handle handle_self, IN TC_Direction direction);




/*
	ֹͣ�ƶ���ԭ�ز�����

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	����ɹ�������true�����򷵻�false��
*/
TC_CLIENT_API bool __stdcall Tc_Stop(IN TC_Handle handle_self);




/*
	ʩ��ħ��

	����ֻ���Լ����ѷ�ħ����Ч���������˶��С�

	����ɹ�������true�����򷵻�false��
	ʧ�ܵ�ԭ������Ǵ�������Ч�ľ������ǰ�Լ�����ʩ��ħ�������ߵ�ǰ��
	�ܶԶԷ�ʩ��ħ����
*/
TC_CLIENT_API bool  __stdcall Tc_Spell(IN TC_Handle handle_self, IN TC_Handle handle_target);


/*
	�Ƿ��ܹ���ʩ��

	handle_self�ǡ��Լ���������ĳ����Ա�ľ���������Ա�ǽ�Ҫʩ�����ˡ�
	������ͨ��Tc_GetHeroHandleȡ�á�

	handle_enemy�ǡ��Է��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetEnemyHandleȡ�á�

	����ܹ���ʩ���򷵻�true�����򷵻�false��
	���Ŀ�����ڷ�ħ����Ŀ���Ѿ�����ĳ��ħ������ħ��Ч����û��ʧЧ��
	Ŀ�����̫Զ����ǰ���ܶ���ʩ����

*/
TC_CLIENT_API bool __stdcall Tc_CanBeSpelled(IN TC_Handle handle_self, IN TC_Handle handle_enemy);




/*
	����ֻ�ܴ������

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	pos_target�Ǵ����Ŀ��λ�á�

	����ɹ������Գ�ȥ���򷵻�true�����򷵻�false��
	����trueʱֻ��ʾ�������˳�ȥ�����ڴ����ܲ��ܳɹ����ǡ�����ȷ�����ġ�
	��Ϊ���ӳ�ȥ֮�����Ҳ��������;���򣬴Ӷ����´���ʧ�ܡ�

	ע�⴫��λ�á��������⡿�������ڲ��ܴ���
*/
TC_CLIENT_API bool  __stdcall Tc_PassBall(IN TC_Handle handle_self, IN TC_Position &pos_target);




/*
	���򣬹����ͽ�ɫ������

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	ball_type��������ͣ�������ÿ����ֻ��һ����ֻ���������ͽ�ɫ������

	����������򷵻�true�����򷵻�false��
*/
TC_CLIENT_API bool  __stdcall Tc_SnatchBall(IN TC_Handle handle_self, IN TC_Ball_Type ball_type);



/*
	ȡ�á��Լ���������ĳ����Ա����Ϣ��

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	p_hero_info��ָ��TC_Hero���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_hero_infoָ��ı����У�����������ڴ�ռ��ǵ���������ġ�
*/
TC_CLIENT_API bool  __stdcall Tc_GetHeroInfo(IN TC_Handle handle_self, IN OUT PTC_Hero p_hero_info);




/*
	ȡ�á��Է���������ĳ����Ա����Ϣ��

	handle_enemy�ǡ��Է��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetEnemyHandleȡ�á�

	p_enemy_info��ָ��TC_Hero���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_enemy_infoָ��ı����У�����������ڴ�ռ��ǵ���������ġ�
*/
TC_CLIENT_API bool  __stdcall Tc_GetEnemyInfo(IN TC_Handle handle_enemy, IN OUT PTC_Hero p_enemy_info);




/*
	ȡ�������Ϣ��

	handle_ball����ľ����������ͨ��Tc_GetBallHandle����ȡ�á�

	p_ball_info��ָ��TC_Ball���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_ball_infoָ��ı����У�����������ڴ�ռ��ǵ���������ġ�
*/
TC_CLIENT_API bool  __stdcall Tc_GetBallInfo(IN TC_Handle handle_ball, IN OUT PTC_Ball p_ball_info);




/*
	ȡ�á��Լ���������ĳ��Ӣ�۵ľ����

	memberָ�����Ǹ���Ա��

	���ض�Ӧ��Ա�ľ����
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetHeroHandle(IN TC_Team_Member member);




/*
	ȡ�á��Է���������ĳ��Ӣ�۵ľ����

	memberָ�����Ǹ���Ա��

	���ض�Ӧ��Ա�ľ����
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetEnemyHandle(IN TC_Team_Member member);




/*
	ȡ��ĳ����ľ����

	ball_typeָ�����Ǹ��򣬱�����ÿ����ֻ��һ����

	����ָ����ľ����
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetBallHandle(IN TC_Ball_Type ball_type);




/*
	ȡ���Լ����ŵ�λ�á�

	handle_self�ǡ��Լ��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	p_own_gate��ָ��TC_Gate���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_own_gateָ��ı����У�����������ڴ�ռ��ǵ���������ġ�
*/
TC_CLIENT_API bool __stdcall Tc_GetOwnGate(IN TC_Handle handle_self, IN OUT PTC_Gate p_own_gate);




/*
	ȡ�öԷ����ŵ�λ�á�

	handle_enemy�ǡ��Է��������е�ĳ����Ա�ľ����������ͨ��
	Tc_GetEnemyHandleȡ�á�

	p_enemy_gate��ָ��TC_Gate���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_enemy_gateָ��ı����У�����������ڴ�ռ��ǵ��������롣

*/
TC_CLIENT_API bool __stdcall Tc_GetEnemyGate(IN TC_Handle handle_enemy, IN OUT PTC_Gate p_enemy_gate);



/*
	ȡ�ñ�����ص���Ϣ

	handle_self�ǡ��Լ���������ĩ����Ա�ľ����������ͨ��
	Tc_GetHeroHandleȡ�á�

	p_game_infoָ��TC_GameInfo���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_game_infoָ��ı����У�����������ڴ�ռ��ǵ���������ġ�
*/
TC_CLIENT_API bool __stdcall Tc_GetGameInfo(TC_Handle handle_self, PTC_GameInfo p_game_info);



/*
	ȡ�ô������λ��

	p_forbidden_areaָ��TC_ForbiddenArea���ͱ�����һ��ָ�롣

	������в�������Ч�򷵻�true�����򷵻�false��
	���ص���Ϣ��p_forbidden_areaָ��ı����У�����������ڴ�ռ��ǵ�����
	����ġ�
*/
TC_CLIENT_API bool __stdcall Tc_GetForbiddenArea(PTC_ForbiddenArea p_forbidden_area);


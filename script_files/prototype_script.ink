// TAG LIST:
// pause: stop until prompted to. 


Welcome to your first day on the job.
To make things easy for you, we have stationed you in a hospital.
In order to make today's quota, you must reap at least 1 (one) soul.
As you may or may not know, 
reapers have the flexibility to choose their own method of collection. 
When you attempt to collect their souls,
Humans on the verge of death are acutely aware of your presence.
Therefore, you may chose to drag your target to the depths
or convince your target to be collected. 
What you choose is up to you. # pause

-> henry

=== henry ===
    ~ temp checked_diary = false
    ~ temp battle_difficulty = 3
    This is your first client.
    If you're in a rush, you may directly apporoach to reap his soul
    Or, you may choose to know more about him to make it easier. 
    { checked_diary:
        DIARY CONTENTS: 
        "Stacy and the grandbabies visited me again. 
        Jack turned 14 yesterday and he starts high school soon... 
        Alice is entering the third grade and she adores her friends. 
        They are both growing so big. Andrew couldn't visit because of his work. 
        He should spend more time with them.
        Stacy looks just like Dee when she was younger. 
        If only Dee could see them now." 
        ~ battle_difficulty = 1
    }
    
    { checked_diary:
        "It is time."
        "'It is time'...? Ah, are you death? I can't go...
        I am not ready. Everyone still needs me...
        But you read my diary so I dont mind"
    - else: 
         "It is time."
        "'It is time'...? Ah, are you death? I can't go...
        I am not ready. Everyone still needs me...
        But you didnt read my diary so I'll put up a fight!"
    } # battle

-> END

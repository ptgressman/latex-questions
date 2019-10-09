import xml.etree.ElementTree as QT

def indent(elem, level=0):
    i = "\n" + level*"  "
    j = "\n" + (level-1)*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for subelem in elem:
            indent(subelem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = j
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = j
    return elem

def QTI_initialize():
    myqti = QT.Element("questestinterop")
    myqti.set("xmlns","http://www.imsglobal.org/xsd/ims_qtiasiv1p2")
    myqti.set("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance")
    myqti.set("xsi:schemaLocation","http://www.imsglobal.org/xsd/ims_qtiasiv1p2 http://www.imsglobal.org/xsd/ims_qtiasiv1p2p1.xsd")
    return myqti

def QTI_new_bank(myqti,ident,title):
    mybank = QT.SubElement(myqti,"objectbank")      # Create Bank
    mybank.set("ident",ident)              # Bank ID
    mybank.set("title",title)              # Bank Title
    mybankmetadata = QT.SubElement(mybank,"qtimetadata")
    mybankmetadata2 = QT.SubElement(mybankmetadata,"qtimetadatafield")
    mybankmetadata3 = QT.SubElement(mybankmetadata2,"fieldlabel")
    mybankmetadata3.text = "bank_name"
    mybankmetadata4 = QT.SubElement(mybankmetadata2,"fieldentry")
    mybankmetadata4.text = title           # Enter the bank title again
    return mybank

def QTI_add_question(mybank,megaquestion):
    ident = megaquestion[0][2]
    title = megaquestion[0][3]
    question_type = megaquestion[0][4]
    statement = megaquestion[0][5]
    item = QT.SubElement(mybank,"item")    # Create Question
    item.set("ident",ident)                # Question ID
    item.set("title",title)                # Question Title
    iteminfo = QT.SubElement(item,"itemmetadata")
    iteminfo2 = QT.SubElement(iteminfo,"qtimetadata")
    iteminfo2a = QT.SubElement(iteminfo2,"qtimetadatafield")
    iteminfo2b = QT.SubElement(iteminfo2a,"fieldlabel")
    iteminfo2b.text = "question_type"
    itemquestiontype = QT.SubElement(iteminfo2a,"fieldentry")
    itemquestiontype.text = question_type    #Type: short_answer_question, file_upload_question
    iteminfo2a = QT.SubElement(iteminfo2,"qtimetadatafield")
    iteminfo2b = QT.SubElement(iteminfo2a,"fieldlabel")
    iteminfo2b.text = "points_possible"
    iteminfo2c = QT.SubElement(iteminfo2a,"fieldentry")
    iteminfo2c.text = "1.0"
    iteminfo2a = QT.SubElement(iteminfo2,"qtimetadatafield")
    iteminfo2b = QT.SubElement(iteminfo2a,"fieldlabel")
    iteminfo2b.text = "original_answer_ids"
    itemanswerids = QT.SubElement(iteminfo2a,"fieldentry")
    itemanswerids.text = ""                    #Enter the IDs of the answers
    for responseitem in megaquestion[1]:
        itemanswerids.text += responseitem[0] + ","
    itemanswerids.text = itemanswerids.text[0:len(itemanswerids.text)-1]
    present = QT.SubElement(item,"presentation")
    presmat = QT.SubElement(present,"material")
    itemstatement = QT.SubElement(presmat,"mattext")
    itemstatement.set("texttype","text/html")
    itemstatement.text = statement        #This is where the question content goes
    if question_type=="multiple_choice_question":
        respstuff1 = QT.SubElement(present,"response_lid")
        respstuff1.set("ident","response1")
        respstuff1.set("rcardinality","Single")
        itemchoices = QT.SubElement(respstuff1,"render_choice") #
        for responseitem in megaquestion[1]:
            responder1 = QT.SubElement(itemchoices,"response_label")# Start here and repeat for all answers
            responder1.set("ident",responseitem[0])                 # Here is where you set answer IDs
            responder2 = QT.SubElement(responder1,"material")       #
            responder3 = QT.SubElement(responder2,"mattext")        #
            responder3.set("texttype","text/html")                  #
            responder3.text = responseitem[1]                       # End here repeat for all answers
    elif question_type=="short_answer_question":
        respstuff1 = QT.SubElement(present,"response_str")
        respstuff1.set("ident","response1")
        respstuff1.set("rcardinality","Single")
        respstuff2 = QT.SubElement(respstuff1,"render_fib")
        respstuff3 = QT.SubElement(respstuff2,"response_label")
        respstuff3.set("ident","answer1")
        respstuff3.set("rshuffle","No")

    ##### File Upload Questions do neither of the above

    action0 = QT.SubElement(item,"resprocessing")
    action1 = QT.SubElement(action0,"outcomes")
    action2 = QT.SubElement(action1,"decvar")
    action2.set("maxvalue","100")
    action2.set("minvalue","0")
    action2.set("varname","SCORE")
    action2.set("vartype","Decimal")

    if (megaquestion[3][0] != ""):
        setcomm = QT.SubElement(action0,"respcondition")        # To give a general comment
        setcomm.set("continue","Yes")                           #
        setcomm1 = QT.SubElement(setcomm,"conditionvar")        #
        setcomm2 = QT.SubElement(setcomm1,"other")              #
        extracomment = QT.SubElement(setcomm,"displayfeedback") # Do these to give general comments
        extracomment.set("feedbacktype","Response")             # Note that you need to set the ID
        extracomment.set("linkrefid","general_fb")        # For general_comments

    givepoints = QT.SubElement(action0,"respcondition")
    givepoints.set("continue","No")
    givepoints1 = QT.SubElement(givepoints,"conditionvar")

    if (question_type=="multiple_choice_question"):
        itemcorrect = QT.SubElement(givepoints1,"varequal")
        itemcorrect.set("respident","response1")            #   Do once for multiple choice, text is ident of
        itemcorrect.text = megaquestion[2][0]               #   correct answer
    elif question_type=="short_answer_question":
        for responseitem in megaquestion[1]:
            itemcorrect = QT.SubElement(givepoints1,"varequal") #   Repeat for short answer, text is correct entry texts
            itemcorrect.set("respident","response1")
            itemcorrect.text = responseitem[1]
    givepoints2 = QT.SubElement(givepoints,"setvar")
    givepoints2.set("action","Set")
    givepoints2.set("varname","SCORE")
    givepoints2.text="100"
    if (megaquestion[3][1] != ""):
        extrapraise = QT.SubElement(givepoints,"displayfeedback")   # Do these to give praise
        extrapraise.set("feedbacktype","Response")          #       Note that you need to set the ID
        extrapraise.set("linkrefid","general_correct_fb")       #       For general_praise
    if megaquestion[3][2] != "":
        setcomm = QT.SubElement(action0,"respcondition")        # To give feedback for incorrect
        setcomm.set("continue","Yes")                           #
        setcomm1 = QT.SubElement(setcomm,"conditionvar")        #
        setcomm2 = QT.SubElement(setcomm1,"other")              #
        extracomment = QT.SubElement(setcomm,"displayfeedback") # Do these to give feedback for incorrect
        extracomment.set("feedbacktype","Response")             # Note that you need to set the ID
        extracomment.set("linkrefid","general_incorrect_fb")    # For general_feedback
    if megaquestion[3][0] != "":
        feedbackitem = QT.SubElement(item,"itemfeedback")      # Do this for each type of feedback
        feedbackitem.set("ident","general_fb")              # or general_comments or general_feedback
        feedbackitem1 = QT.SubElement(feedbackitem,"flow_mat")  #
        feedbackitem2 = QT.SubElement(feedbackitem1,"material") #
        feedbackstatement = QT.SubElement(feedbackitem2,"mattext")
        feedbackstatement.set("texttype","text/html")           #
        feedbackstatement.text = megaquestion[3][0]         #
    if megaquestion[3][1] != "":
        feedbackitem = QT.SubElement(item,"itemfeedback")      # Do this for each type of feedback
        feedbackitem.set("ident","general_correct_fb")              # or general_comments or general_feedback
        feedbackitem1 = QT.SubElement(feedbackitem,"flow_mat")  #
        feedbackitem2 = QT.SubElement(feedbackitem1,"material") #
        feedbackstatement = QT.SubElement(feedbackitem2,"mattext")
        feedbackstatement.set("texttype","text/html")           #
        feedbackstatement.text = megaquestion[3][1]         #
    if megaquestion[3][2] != "":
        feedbackitem = QT.SubElement(item,"itemfeedback")      # Do this for each type of feedback
        feedbackitem.set("ident","general_incorrect_fb")              # or general_comments or general_feedback
        feedbackitem1 = QT.SubElement(feedbackitem,"flow_mat")  #
        feedbackitem2 = QT.SubElement(feedbackitem1,"material") #
        feedbackstatement = QT.SubElement(feedbackitem2,"mattext")
        feedbackstatement.set("texttype","text/html")           #
        feedbackstatement.text = megaquestion[3][2]         #

def QTI_generate_XML(myqti):
    result = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + QT.tostring(myqti)
    return result

def QTI_generate_manifest(outeridentifier,inneridentifier,local_filename,other_resources):
    packingslip = QT.Element("manifest")
    packingslip.set("identifer",outeridentifier) # It's not clear this makes any substantive difference
    packingslip.set("xmlns","http://www.imsglobal.org/xsd/imsccv1p1/imscp_v1p1")
    packingslip.set("xmlns:lom","http://ltsc.ieee.org/xsd/imsccv1p1/LOM/resource")
    packingslip.set("xmlns:imsmd","http://www.imsglobal.org/xsd/imsmd_v1p2")
    packingslip.set("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance")
    packingslip.set("xsi:schemaLocation","http://www.imsglobal.org/xsd/imsccv1p1/imscp_v1p1 http://www.imsglobal.org/xsd/imscp_v1p1.xsd http://ltsc.ieee.org/xsd/imsccv1p1/LOM/resource http://www.imsglobal.org/profile/cc/ccv1p1/LOM/ccv1p1_lomresource_v1p0.xsd http://www.imsglobal.org/xsd/imsmd_v1p2 http://www.imsglobal.org/xsd/imsmd_v1p2p2.xsd")
    moreinfo = QT.SubElement(packingslip,"metadata")
    moreinfo1 = QT.SubElement(moreinfo,"schema")
    moreinfo1.text="IMS Content"
    moreinfo2 = QT.SubElement(moreinfo,"schemaversion")
    moreinfo2.text="1.1.3"
    orginfo = QT.SubElement(packingslip,"organizations")
    resinf0 = QT.SubElement(packingslip,"resources")
    manifest = QT.SubElement(resinf0,"resource")
    manifest.set("href",local_filename)
    manifest.set("identifier",inneridentifier)   # This might matter
    manifest.set("type","imsqti_xmlv1p2")
    QT.SubElement(manifest,"file").set("href",local_filename) # This does matter; repeat for all files
    for m in other_resources:
        QT.SubElement(manifest,"file").set("href",m)
    result = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + QT.tostring(packingslip)
    return result

#Here's a demo implementation
#megaquestion = [["B01","Thisbank","Q01","qtitle1","multiple_choice_question","multiple choice statement"],[["R01","statement1"],["R02","statement2"]],["R01"],["comments","praise","feedback statement"]]
#megaquestion2 = [["B01","Thisbank","Q02","qtitle2","short_answer_question","short answer statement"],[["R01","SAstatement1"],["R02","SAstatement2"]],[],["comments","praise","feedback statement"]]
#megaquestion3 = [["B01","Thisbank","Q03","qtitle3","file_upload_question","file upload statement"],[],[],["comments","",""]]
#myqti = QTI_initialize()
#bank = QTI_new_bank(myqti,"bankident","banktitle")
#QTI_add_question(bank,megaquestion)
#QTI_add_question(bank,megaquestion2)
#QTI_add_question(bank,megaquestion3)
#
#import zipfile
#tinyfile = zipfile.ZipFile("testzip.zip","w")
#tinyfile.writestr("thexmlfile.xml",QTI_generate_XML(myqti))
#tinyfile.writestr("imsmanifest.xml",QTI_generate_manifest("autogen","thefile","thexmlfile.xml",[]))
#tinyfile.close()

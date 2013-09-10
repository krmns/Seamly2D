#include "vmodelingarc.h"
#include <QMenu>
#include "container/calculator.h"

VModelingArc::VModelingArc(VDomDocument *doc, VContainer *data, qint64 id, Tool::Enum typeCreation,
                           QGraphicsItem *parent):VModelingTool(doc, data, id), QGraphicsPathItem(parent),
    dialogArc(QSharedPointer<DialogArc>()){
    VArc arc = data->GetModelingArc(id);
    QPainterPath path;
    path.addPath(arc.GetPath());
    path.setFillRule( Qt::WindingFill );
    this->setPath(path);
    this->setPen(QPen(Qt::black, widthHairLine));
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setAcceptHoverEvents(true);

    if(typeCreation == Tool::FromGui){
        AddToFile();
    }
}

void VModelingArc::setDialog(){
    Q_ASSERT(!dialogArc.isNull());
    if(!dialogArc.isNull()){
        VArc arc = VAbstractTool::data.GetModelingArc(id);
        dialogArc->SetCenter(arc.GetCenter());
        dialogArc->SetRadius(arc.GetFormulaRadius());
        dialogArc->SetF1(arc.GetFormulaF1());
        dialogArc->SetF2(arc.GetFormulaF2());
    }
}

VModelingArc* VModelingArc::Create(QSharedPointer<DialogArc> &dialog, VDomDocument *doc, VContainer *data){
    qint64 center = dialog->GetCenter();
    QString radius = dialog->GetRadius();
    QString f1 = dialog->GetF1();
    QString f2 = dialog->GetF2();
    return Create(0, center, radius, f1, f2, doc, data, Document::FullParse, Tool::FromGui);
}

VModelingArc* VModelingArc::Create(const qint64 _id, const qint64 &center, const QString &radius,
                                   const QString &f1, const QString &f2, VDomDocument *doc,
                                   VContainer *data, Document::Enum parse, Tool::Enum typeCreation){
    VModelingArc *toolArc = 0;
    qreal calcRadius = 0, calcF1 = 0, calcF2 = 0;

    Calculator cal(data);
    QString errorMsg;
    qreal result = cal.eval(radius, &errorMsg);
    if(errorMsg.isEmpty()){
        calcRadius = result*PrintDPI/25.4;
    }

    errorMsg.clear();
    result = cal.eval(f1, &errorMsg);
    if(errorMsg.isEmpty()){
        calcF1 = result;
    }

    errorMsg.clear();
    result = cal.eval(f2, &errorMsg);
    if(errorMsg.isEmpty()){
        calcF2 = result;
    }

    VArc arc = VArc(data->DataModelingPoints(), center, calcRadius, radius, calcF1, f1, calcF2, f2 );
    qint64 id = _id;
    if(typeCreation == Tool::FromGui){
        id = data->AddModelingArc(arc);
    } else {
        data->UpdateModelingArc(id, arc);
        if(parse != Document::FullParse){
            QMap<qint64, VDataTool*>* tools = doc->getTools();
            VDataTool *tool = tools->value(id);
            if(tool != 0){
                tool->VDataTool::setData(data);
                data->IncrementReferens(id, Scene::Arc, Draw::Modeling);
            }
        }
    }
    data->AddLengthArc(data->GetNameArc(center,id, Draw::Modeling), arc.GetLength());
    data->IncrementReferens(center, Scene::Point, Draw::Modeling);
    if(parse == Document::FullParse){
        toolArc = new VModelingArc(doc, data, id, typeCreation);
        QMap<qint64, VDataTool*>* tools = doc->getTools();
        tools->insert(id,toolArc);
    }
    return toolArc;
}

void VModelingArc::FullUpdateFromFile(){
    RefreshGeometry();
}

void VModelingArc::FullUpdateFromGui(int result){
    if(result == QDialog::Accepted){
        QDomElement domElement = doc->elementById(QString().setNum(id));
        if(domElement.isElement()){
            domElement.setAttribute("center", QString().setNum(dialogArc->GetCenter()));
            domElement.setAttribute("radius", dialogArc->GetRadius());
            domElement.setAttribute("angle1", dialogArc->GetF1());
            domElement.setAttribute("angle2", dialogArc->GetF2());
            emit FullUpdateTree();
        }
    }
    dialogArc.clear();
}

void VModelingArc::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    ContextMenu(dialogArc, this, event);
}

void VModelingArc::AddToFile(){
    VArc arc = VAbstractTool::data.GetModelingArc(id);
    QDomElement domElement = doc->createElement("arc");

    AddAttribute(domElement, "id", id);
    AddAttribute(domElement, "type", "simple");
    AddAttribute(domElement, "center", arc.GetCenter());
    AddAttribute(domElement, "radius", arc.GetFormulaRadius());
    AddAttribute(domElement, "angle1", arc.GetFormulaF1());
    AddAttribute(domElement, "angle2", arc.GetFormulaF2());

    AddToModeling(domElement);
}

void VModelingArc::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        emit ChoosedTool(id, Scene::Arc);
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void VModelingArc::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    Q_UNUSED(event);
    this->setPen(QPen(currentColor, widthMainLine));
}

void VModelingArc::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    Q_UNUSED(event);
    this->setPen(QPen(currentColor, widthHairLine));
}

void VModelingArc::RefreshGeometry(){
    VArc arc = VAbstractTool::data.GetModelingArc(id);
    QPainterPath path;
    path.addPath(arc.GetPath());
    path.setFillRule( Qt::WindingFill );
    this->setPath(path);
}
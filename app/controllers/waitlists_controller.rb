class WaitlistsController < ApplicationController
  # GET /waitlists
  # GET /waitlists.xml
  def index
    @waitlists = Waitlist.all

    respond_to do |format|
      format.html # index.html.erb
      format.xml  { render :xml => @waitlists }
    end
  end

  # GET /waitlists/1
  # GET /waitlists/1.xml
  def show
    @waitlist = Waitlist.find(params[:id])

    respond_to do |format|
      format.html # show.html.erb
      format.xml  { render :xml => @waitlist }
    end
  end

  # GET /waitlists/new
  # GET /waitlists/new.xml
  def new
    @waitlist = Waitlist.new

    respond_to do |format|
      format.html # new.html.erb
      format.xml  { render :xml => @waitlist }
    end
  end

  # GET /waitlists/1/edit
  def edit
    @waitlist = Waitlist.find(params[:id])
  end

  # POST /waitlists
  # POST /waitlists.xml
  def create
    @waitlist = Waitlist.new(params[:waitlist])

    respond_to do |format|
      if @waitlist.save

        format.html { redirect_to(@waitlist) }
        format.xml  { render :xml => @waitlist, :status => :created, :location => @waitlist }
      else
        format.html { render :action => "new" }
        format.xml  { render :xml => @waitlist.errors, :status => :unprocessable_entity }
      end
    end
  end

  # PUT /waitlists/1
  # PUT /waitlists/1.xml
  def update
    @waitlist = Waitlist.find(params[:id])

    respond_to do |format|
      if @waitlist.update_attributes(params[:waitlist])
        flash[:notice] = 'Waitlist was successfully updated.'
        format.html { redirect_to(@waitlist) }
        format.xml  { head :ok }
      else
        format.html { render :action => "edit" }
        format.xml  { render :xml => @waitlist.errors, :status => :unprocessable_entity }
      end
    end
  end

  # DELETE /waitlists/1
  # DELETE /waitlists/1.xml
  def destroy
    @waitlist = Waitlist.find(params[:id])
    @waitlist.destroy

    respond_to do |format|
      format.html { redirect_to(waitlists_url) }
      format.xml  { head :ok }
    end
  end
end
